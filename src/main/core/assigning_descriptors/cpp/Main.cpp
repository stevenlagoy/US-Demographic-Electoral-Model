#include <map>
#include <unordered_map>
#include <string>
#include <set>
#include <array>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <windows.h>
#include <fstream>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;
#include <sstream>
#include <random>
#include <iterator>
#include <chrono>
#include <iomanip>

#include "Descriptor.h"
#include "County.h"

using namespace std;

#define MAX_DESCRIPTORS 300
#define MAX_PERMUTE_CHANCE 0.1f

void normalize(vector<double>& vec) {
    double sum = accumulate(vec.begin(), vec.end(), 0.0f);
    if (sum == 0.0f) return;
    for (auto &v : vec) v /= sum;
}

double compareDemographics(const vector<double>& expected, const vector<double>& actual, string method = "l1") {

    // Check that vectors have same length
    if (expected.size() != actual.size()) {
        cerr << "Compared vectors have different sizes: " << to_string(expected.size()) << ", " << to_string(actual.size()) << "." << endl; 
        return 0.0;
    }

    // Copy vectors
    vector<double> e = expected;
    vector<double> a = actual;

    // Normalize vectors
    normalize(e);
    normalize(a);

    // Check if population disparity
    if (accumulate(a.begin(), a.end(), 0.0) == 0.0 &&
        accumulate(e.begin(), e.end(), 0.0) != 0.0) {
        return 0.0;
    }

    if (method == "l1") { // L1 Norm - Sum of absolute differences (Manhattan Distance)
        vector<double> distances = vector<double>();
        for (size_t i = 0; i < e.size(); i++) {
            distances.emplace_back(abs(e.at(i) - a.at(i)));
        }
        double dist = accumulate(distances.begin(), distances.end(), 0.0);
        return 1 - dist / 2; // Normalize to [0, 1]
    }
    else if (method == "l2") { // L2 Norm - Eucledian distance
        vector<double> distances = vector<double>();
        for (size_t i = 0; i < e.size(); i++) {
            distances.emplace_back(pow(e.at(i) - a.at(i), 2));
        }
        double dist = sqrt(accumulate(distances.begin(), distances.end(), 0.0));
        return 1 - dist / 2; // Normalize to [0, 1]
    }
    else if (method == "cosine") { // Cosine Similarities - Dot product
        double dot = inner_product(e.begin(), e.end(), a.begin(), 0.0);
        return dot;
    }
    else if (method == "js") { // Jensen-Shannon Divergence
        auto kl = [](vector<double> p, vector<double> q) { // Kullback-Leibler
            vector<double> d = vector<double>();
            for(size_t i = 0; i < p.size(); ++i) {
                if (p[i] && q[i])
                    d.emplace_back(p[i] * log2(p[i]/q[i]));
            }
            double sum = accumulate(d.begin(), d.end(), 0.0);
            return sum;
        };
        vector<double> m = vector<double>();
        for (size_t i = 0; i < e.size(); ++i) {
            m.emplace_back((e.at(i) + a.at(i))/2);
        }
        double js = (kl(e, m) + kl(a, m)) / 2; // Get J-S divergence
        double sim = 1 - js; // Invert
        return clamp(sim, 0.0, 1.0);
    }
    else { // Invalid method
        throw invalid_argument("Unknown method: " + method);
    }
}

unordered_map<County*, double> countiesScores;
double scoreCounty(County& c) {
    double score = compareDemographics(c.getDemographics(), c.descriptorDemographics());
    countiesScores[&c] = score;
    return score;
}

double score() {
    size_t num = countiesScores.size();
    if (num == 0) return 0.0f;
    double avgScore = 0.0f;
    for (const auto& s : countiesScores) {
        avgScore += s.second / num;
    }
    return avgScore;
}

vector<string> listDirectories(const string& path) {
    vector<string> dirs;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) return dirs;
    
    do {
        string name = fd.cFileName;
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..") {
            dirs.push_back(name);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return dirs;
}

vector<string> listFiles(const string& path) {
    vector<string> files;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) return files;

    do {
        string name = fd.cFileName;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(name);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return files;
}

void flattenJson(const json& j, map<string, double>& result, const string& parentKey = "", const string& sep = "->") {
    for (auto& [k, v] : j.items()) {
        string newKey = parentKey.empty() ? k : parentKey + sep + k;

        if (v.is_object()) {
            flattenJson(v, result, newKey, sep); // Recurse for nesting
        }
        else if (v.is_number_float() || v.is_number_integer()) {
            result[newKey] = v.get<double>();
        }
        else {
            throw runtime_error("Unsupported type for key " + newKey);
        }

    }
}

// https://stackoverflow.com/a/16421677
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    uniform_int_distribution<> dis(0, distance(start, end) - 1);
    advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static random_device rd;
    static mt19937 gen(
        static_cast<unsigned long>(
            chrono::system_clock::now().time_since_epoch().count()
        )
    );
    return select_randomly(start, end, gen);
}

vector<County> counties;
unordered_map<string, size_t> demoIndex;
vector<string> demographics;

void readCounties() {
    counties.clear();
    demoIndex.clear();
    demographics.clear();

    struct RawCounty {
        string name;
        string state;
        int population;
        map<string, double> demos;
    };
    vector <RawCounty> raw;

    // Gather all demographic blocs
    for (const string& state : listDirectories("..\\..\\..\\resources")) {
        string countyDir = "..\\..\\..\\resources\\" + state + "\\counties";
        for (const string& file : listFiles(countyDir)) {
            size_t dot = file.find('.');
            if (dot == string::npos || !all_of(file.begin(), file.begin() + dot, ::isdigit)) continue;
            
            // Open file
            ifstream f(countyDir + "\\" + file);
            if (!f.is_open()) continue;
            // Read into buffer
            stringstream buffer;
            buffer << f.rdbuf();
            f.close();
            // Read JSON
            json j = json::parse(buffer.str());

            RawCounty rc;
            rc.name = j.value("name", string());
            rc.state = j.value("state", string());
            rc.population = j.value("population", 0);
            flattenJson(j["demographics"], rc.demos);
            
            // Add demographics keys to the global vector
            for (const auto& [key, val] : rc.demos) {
                if (!demoIndex.count(key)) {
                    demoIndex[key] = demographics.size();
                    demographics.push_back(key);
                }
            }

            raw.push_back(move(rc));
        }
    }

    // Create counties
    counties.reserve(raw.size());
    for (const auto& rc : raw) {
        vector<double> demosVec(demographics.size(), 0.0f);
        for (const auto& [key, val] : rc.demos) {
            auto it = demoIndex.find(key);
            if (it != demoIndex.end()) {
                demosVec[it->second] = val;
            }
        }
        counties.emplace_back(rc.name, rc.state, rc.population, demosVec);
    }
}

vector<Descriptor> descriptors;
vector<size_t> modifiable;

void initialize() {

    // Create counties
    readCounties();

    // Assign base descriptors (nation, state) to each county

    vector<double> emptyEffects(demographics.size(), 0.0f);
    descriptors.reserve(1 + counties.size() + MAX_DESCRIPTORS);

    // Create and store nation descriptor
    descriptors.emplace_back("Nation", emptyEffects, true);
    Descriptor* nation = &descriptors.back();

    map<string, size_t> states;
    for (County& c : counties) {
        // Add the nation descriptor to every county
        c.addDescriptor(nation);

        // Add the state descriptor to each county
        // Create state descriptor if not already made
        if (!states.count(c.getState())) {
            descriptors.emplace_back(c.getState(), emptyEffects, true);
            states[c.getState()] = descriptors.size() - 1;
        }
        c.addDescriptor(&descriptors[states.at(c.getState())]);
    }

    // Create blank descriptors up to MAX_DESCRIPTORS
    for (int i = 0; descriptors.size() <= MAX_DESCRIPTORS; ++i) {
        Descriptor d("Descriptor " + to_string(i), emptyEffects);
        descriptors.emplace_back(d);
        modifiable.push_back(descriptors.size() - 1);
    }

    // Pre-calculate counties
    for (auto& c : counties) {
        c.recalculate();
        scoreCounty(c);
    }
}

struct Change {
    function<void()> undo_fn;
    Change(function<void()> fn) : undo_fn(fn) {}
    void undo() { if (undo_fn) undo_fn(); }
};

Change permuteDescriptors() {
    if (descriptors.empty()) {
        return Change([](){});
    }
    // Select a descriptor to modify
    Descriptor* descriptor = &*select_randomly(descriptors.begin(), descriptors.end());

    auto effects = descriptor->getEffects();
    if (effects.empty()) {
        return Change([](){});
    }
    auto it = select_randomly(effects.begin(), effects.end());
    size_t index = static_cast<size_t>(std::distance(effects.begin(), it));
    double old_value = *it;
    // Choose how much to modify
    static std::mt19937 gen(random_device{}());
    uniform_real_distribution<double> dist(-MAX_PERMUTE_CHANCE, MAX_PERMUTE_CHANCE);
    double mod = dist(gen);
    double new_value = clamp(old_value + mod, 0.0, 1.0);
    // Make the change
    descriptor->setEffect(index, new_value);
    // Recalculate scores
    vector<County*> affected;
    for (auto& c : counties) {
        if (c.hasDescriptor(descriptor)) {
            c.recalculate();
            scoreCounty(c);
            affected.push_back(&c);
        }
    }
    return Change([descriptor, index, old_value, affected]() mutable {
        descriptor->setEffect(index, old_value);
        for (auto* c : affected) {
            c->recalculate();
            scoreCounty(*c);
        }
    });
}

Change permuteCounties() {
    if (counties.empty() || descriptors.empty()) {
        return Change([](){});
    }
    // Select a county to modify
    County* county = &*select_randomly(counties.begin(), counties.end());
    // Select a modifiable descriptor
    size_t idx = *select_randomly(modifiable.begin(), modifiable.end());
    Descriptor* descriptor = &descriptors[idx];
    // Determine whether to add or remove
    bool had_descriptor = county->hasDescriptor(descriptor);
    if (had_descriptor) county->removeDescriptor(descriptor);
    else county->addDescriptor(descriptor);
    scoreCounty(*county);

    return Change([county, descriptor, had_descriptor]() mutable {
        if (had_descriptor) {
            county->addDescriptor(descriptor);
        }
        else {
            county->removeDescriptor(descriptor);
        }
        county->recalculate();
        scoreCounty(*county);
    });
}

void run() {
    double prev_score = 0, new_score;
    Change change([](){});
    size_t iter{0};
    while (iter++ < 100'000LL) {
        // cout << iter << endl;
        change = permuteDescriptors();
        new_score = score();
        if (new_score < prev_score) { // Change was worse
            change.undo();
        }
        else { // Change to descriptors was better
            prev_score = new_score;
            cout << iter << " D " << setprecision(18) << new_score << endl;
        }

        change = permuteCounties();
        new_score = score();
        if (new_score < prev_score) { // Change was worse
            change.undo();
        }
        else { // Change to counties was better
            prev_score = new_score;
            cout << iter << " C " << setprecision(18) << new_score << endl;
        }

        if ((iter & 0x3FF) == 0) cout.flush(); // Flush periodically
    }
}

int main() {

    cout << "Initializing..." << endl;
    initialize();
    cout << "Initialization successful" << endl;
    run();

    for (const auto& it : counties) {
        cout << it << endl;
    }
    for (const auto& it : demographics) {
        cout << it << ", ";
    }
    cout << ";\n";
    for (const auto& it : descriptors) {
        cout << it << endl;
    }

    return 0;
}