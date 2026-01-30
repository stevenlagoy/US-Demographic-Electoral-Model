#include "Constants.h"
#include "County.h"
#include "Descriptor.h"
#include "Utils.h"

#include <chrono>
#include <cmath>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <format>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <thread>
#include <mutex>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;

using namespace std;

#define RESOURCES_DIR "../../../resources/2020/"
#define LOGS_DIR "../../../../../logs/"
#define NATION_FILE RESOURCES_DIR "nation.json"
#define LOG_FILE LOGS_DIR "cpplog.json"

struct Simulation {

    struct Change {
        function<void()> undo_fn;
        Change(function<void()> fn) : undo_fn(fn) {}
        void undo() { if (undo_fn) undo_fn(); }
    };

    uint32_t nationalPopulation = 0;
    uint64_t iter = 0;
    uint32_t tries = 0;
    uint16_t threadNum = 0;
    double temperature = STARTING_TEMPERATURE;
    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    vector<unique_ptr<Descriptor>> descriptors;
    vector<unique_ptr<County>> counties;
    unordered_map<string, County*> fipsToCounty;
    array<string, NUMBER_DEMOGRAPHICS> demographicNames;
    bool _INITIALIZED = false;

    Simulation() = default;
    ~Simulation() = default;

    // Copy constructor
    Simulation(const Simulation& other)
        : nationalPopulation(other.nationalPopulation),
          demographicNames(other.demographicNames)
    {
        descriptors.reserve(other.descriptors.size());
        for (const auto& d : other.descriptors) {
            descriptors.push_back(make_unique<Descriptor>(*d));
        }
        
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            counties.push_back(make_unique<County>(*c));
        }

        for (const auto& d : descriptors) {
            d->setCountiesPtr(&counties);
        }
        for (const auto& c : counties) {
            c->setDescriptorsPtr(&descriptors);
        }

        assert(this->descriptors.size() == other.descriptors.size());
        assert(this->demographicNames.size() == other.demographicNames.size() && this->demographicNames.size() == NUMBER_DEMOGRAPHICS);
        assert(this->counties.size() == other.counties.size());
    }

    // Deep copy assignment
    Simulation& operator=(const Simulation& other) {
        if (this == &other) return *this;
        nationalPopulation = other.nationalPopulation;
        descriptors.clear();
        descriptors.reserve(other.descriptors.size());
        for (const auto& d : other.descriptors) {
            descriptors.push_back(make_unique<Descriptor>(*d));
        }
        demographicNames = other.demographicNames;
        counties.clear();
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            counties.push_back(make_unique<County>(*c));
        }
        assert(this->descriptors.size() == other.descriptors.size());
        assert(this->demographicNames.size() == other.demographicNames.size() && this->demographicNames.size() == NUMBER_DEMOGRAPHICS);
        assert(this->counties.size() == other.counties.size());
        return *this;
    }

    // Move semantics
    Simulation(Simulation&&) noexcept = default;
    Simulation& operator=(Simulation&&) noexcept = default;

    void initialize();
    void createCounties();
    void createDescriptors();
    void run();
    Change iteration();
    void cleanup();

    double score();
    double scoreAccuracy();
    double scoreSpecificity();
    double scoreParsimony();
    double scoreLocality();

    string to_string();
    json to_json();
};

atomic<bool> stopRequested = false;
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        stopRequested = true;
        return TRUE;
    }
    return FALSE;
}

ThreadSafeLogger logger;

int main(int argc, char* argv[]) {

    cout << HIDE_CURSOR;

    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [-debug]";
        return -1;
    }
    bool isDebug = false;
    if (argc == 2 && (string(argv[1]) == "-d" || string(argv[1]) == "-debug")) {
        isDebug = true;
    }

    // Add console handler for user interrupt
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Error: Coult not set control handler." << endl;
        return 1;
    }


    // Build base simulation with const county data
    Simulation baseSim;
    logger.logLine("Initializing base model...");
    baseSim.initialize();

    // Clone sim into different threads / workers
    mutex bestMutex;
    double bestScore = 0.0;
    json bestSimJson;
    const uint8_t workers = clamp(thread::hardware_concurrency(), MIN_THREADS, isDebug ? MAX_DEBUG_THREADS : MAX_THREADS);
    vector<thread> threads(workers);
    logger.logLine("Running simulation on " + ::to_string(workers) + " threads...");
    for (auto w = 0; w < workers; ++w) {
        threads.emplace_back([&, w]() {
            try {
                Simulation sim = baseSim; // Copy base simulation
                sim.threadNum = w;

                logger.logLine("[" + ::to_string(sim.threadNum) + "] Running...");
                sim.run();
                double s = sim.score();
                {
                    lock_guard<mutex> lock(bestMutex);
                    if (s > bestScore) {
                        bestScore = s;
                        bestSimJson = sim.to_json();
                        // Write to log file
                        ofstream ofs(LOG_FILE);
                        ofs << bestSimJson.dump(4);
                        ofs.close();
                    }
                }
            }
            catch (const exception& ex) {
                logger.logLine("[", w, "] Exception: ", ex.what());
            }
            catch (...) {
                logger.logLine("[", w, "] Unknwon exception encountered");
            }
        });
    }
    
    // Join threads
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // Print best simulation
    if (bestScore > 0.0) {
        ostringstream oss;
        oss << setprecision(12) << bestScore;
        logger.logLine("Best score across workers: " + oss.str() + ". Saved to " + LOG_FILE);
    }
    else {
        logger.logLine("No successful simulations completed.");
    }

    return 0;
}

void Simulation::initialize() {
    startTime = chrono::steady_clock::now();
    auto initStart = chrono::system_clock::now();
    time_t start_c = chrono::system_clock::to_time_t(initStart);
    ostringstream oss;
    oss << put_time(localtime(&start_c), "%T");
    logger.logLine("Beginning initialization at " + oss.str());
    oss.clear();
    
    createCounties();
    createDescriptors();

    auto initEnd = chrono::system_clock::now();
    time_t end_c = chrono::system_clock::to_time_t(initEnd);
    auto msElapsed = chrono::duration_cast<chrono::milliseconds>(initEnd - initStart).count();
    oss << put_time(localtime(&end_c), "%T");
    logger.logLine("Successfully initialized at " + oss.str() + " (" + ::to_string(msElapsed) + "ms elapsed)");
}

void Simulation::createCounties() {
    counties.clear();

    // Create demographics
    // Use nation.json to find demographic keys, which are consistent with all county datafiles
    json j = freadJson(NATION_FILE);
    map<string, double> demosMap;
    flattenJson(j["demographics"], demosMap);
    size_t demosMade{0};
    for (const auto& d : demosMap) {
        string key = d.first;
        demographicNames[demosMade++] = key;
    }
    assert(demosMade == NUMBER_DEMOGRAPHICS);

    // Create counties
    size_t countiesMade{0};
    vector<vector<string>> countiesNeighborsFIPS; 
    for (const string& state : listDirectories(RESOURCES_DIR)) {
        string countyDir = RESOURCES_DIR + state + "/counties/";
        for (const string& fileName : listFiles(countyDir)) {
            // Verify file name: contains only digits before ".json"
            if (fileName.find('.') == string::npos || !all_of(fileName.begin(), fileName.begin() + fileName.find('.'), ::isdigit)) continue;

            json j = freadJson(countyDir + "\\" + fileName);

            countiesNeighborsFIPS.emplace_back(j["neighbors"]);

            map<string, double> demosMap;
            flattenJson(j["demographics"], demosMap);
            // Map demographics onto known demo indices
            array<double, NUMBER_DEMOGRAPHICS> demosArr;
            for (const auto& d : demosMap) {
                string key = d.first;
                double val = d.second;
                const auto& it = find(demographicNames.cbegin(), demographicNames.cend(), key);
                if (it == demographicNames.cend()) {
                    logger.logLine("Could not find demographic name \"" + key + "\" when reading file " + fileName);
                    continue;
                }
                size_t idx = distance(demographicNames.cbegin(), it);
                demosArr[idx] = val;
            }

            counties.emplace_back(make_unique<County>(
                j["name"],
                j["FIPS"],
                countiesMade,
                j["population"],
                demosArr,
                &(this->descriptors)
            ));
            countiesMade++;
        }
    }
    // Resolve neighbors
    assert(countiesNeighborsFIPS.size() == counties.size());
    for (size_t cIdx = 0; cIdx < countiesNeighborsFIPS.size(); ++cIdx) {
        // Loop through counties
        auto& county = counties[cIdx];
        const auto& neighborsFIPS = countiesNeighborsFIPS[cIdx];
        for (const auto& neighborFIPS : neighborsFIPS) {
            // Loop through county's neighbors
            const auto ncIt = find_if(counties.cbegin(), counties.cend(), [&neighborFIPS](const auto& c) { return c->getFIPS() == neighborFIPS; });
            if (ncIt == counties.cend()) {
                logger.logLine("Could not find county with FIPS: " + neighborFIPS);
                continue;
            }
            size_t ncIdx = static_cast<size_t>(distance(counties.cbegin(), ncIt)); // Always positive, so static_cast is OK
            county->addNeighbor(ncIdx);
            (*ncIt)->addNeighbor(cIdx);
        }
    }
}

void Simulation::createDescriptors() {

    size_t descriptorsMade{0};

    // Create national descriptor
    vector<size_t> memberCountiesIndices;
    for (const auto& c : counties) {
        memberCountiesIndices.push_back(c->getIndex());
    }
    descriptors.emplace_back(make_unique<Descriptor>(
        "$$USA",
        descriptorsMade,
        &(this->counties),
        memberCountiesIndices,
        false
    ));
    descriptorsMade++;

    // Create state descriptors
    map<string, size_t> FIPSToIndex;
    for (const auto& it : stateFIPSToName) {
        string FIPS = it.first;
        string name = it.second;
        string abbr = stateNameToAbbr[name];
        FIPSToIndex[FIPS] = descriptorsMade;
        vector<size_t> memberCountiesIndices;
        for (const auto& c : counties) {
            if (c->getStateFIPS() == FIPS)
                memberCountiesIndices.push_back(c->getIndex());
        }
        descriptors.emplace_back(make_unique<Descriptor>(
            "$" + abbr,
            descriptorsMade,
            &(this->counties),
            memberCountiesIndices,
            false
        ));
        descriptorsMade++;
    }

    // Create initial singleton descriptors
    for (const auto& c : counties) {
        vector<size_t> singleton{c->getIndex()};
        descriptors.emplace_back(make_unique<Descriptor>(
            "DESCRIPTOR_" + ::to_string(descriptorsMade),
            descriptorsMade,
            &(this->counties),
            singleton
        ));
        descriptorsMade++;
    }
}

void Simulation::run() {

    Change ch([](){});
    double prevScore = 0, newScore = 0;
    for (iter = 0; iter < MAX_ITERATIONS && tries < MAX_TRIES && !stopRequested; ++iter) {
        temperature = clamp(temperature - TEMPERATURE_STEP, 0.0, 1.0);

        ch = this->iteration();

        newScore = score();
        if (newScore < prevScore) { // The change made the simulation worse
            if (randomChance(temperature)) { // Temperature check
                // Keep the change
                prevScore = newScore;
                tries = 0;
            }
            else {
                // Undo the change
                ch.undo();
                ++tries;
            }
        }
        else { // Keep the change
            prevScore = newScore;
            tries = 0;
        }

        this->cleanup();

        // Print details
        if (!!(iter % PRINT_TSTATUS_EVERY)) {
            logger.logLine(this->to_string());
        }

    }

    // Print termination details
    ostringstream oss;
    oss << setfill('0') << setw(2) << threadNum;
    logger.log("[", oss.str(), "] ");
    if (stopRequested) {
        logger.logLine("Simulation interrupted by user.");
    }
    else if (tries >= MAX_TRIES) {
        logger.logLine(::to_string(MAX_TRIES), " iterations without improvement. Dropping out.");
    }
    else {
        logger.logLine("Simulation limit of ", ::to_string(MAX_ITERATIONS), " iterations reached.");
    }
}

Simulation::Change Simulation::iteration() {
    
    // Choose a county
    if (counties.empty()) {
        logger.logLine("[" + ::to_string(threadNum) + "] Counties is empty in iteration()");
    }
    auto& county = randomItem(counties);
    // Choose whether to add or remove a descriptor
    vector<size_t> modifiableDescriptorIndices;
    if (randomChance(ADD_DESCRIPTOR_TO_COUNTY_CHANCE)) { // Add
        // Get the modifiable descriptors from the simulation
        for (const auto& d : this->descriptors) {
            if (d->isMembershipModifiable())
                modifiableDescriptorIndices.push_back(d->getIndex());
        }
    }
    else { // Remove
        // Get the modifiable descriptors from the county's membership
        for (const auto& dIdx : county->getDescriptorIndices()) {
            if (this->descriptors[dIdx]->isMembershipModifiable())
                modifiableDescriptorIndices.push_back(dIdx);
        }
    }
    // Choose a descriptor
    if (modifiableDescriptorIndices.size() == 0) return Change([](){}); // No applicable modifiable descriptors were found
    const auto& dIdx = randomItem(modifiableDescriptorIndices);
    const auto& descriptor = descriptors[dIdx];
    // Add or remove the county
    county->addOrRemoveDescriptor(dIdx);
    descriptor->addOrRemoveMemberCounty(county->getIndex());

    return Change([&]() mutable {
        county->addOrRemoveDescriptor(dIdx);
        descriptor->addOrRemoveMemberCounty(county->getIndex());
    });
}

void Simulation::cleanup() {
    // Find and delete any empty descriptors
    erase_if(descriptors, [](auto& d) {
        return d->getMemberCountiesIndices().empty();
    }); // Smart pointer also deletes the descriptor
}

double Simulation::score() {

    double totalScore{0};

    totalScore += this->scoreAccuracy() * ACCURACY_SCORE_WEIGHT;
    totalScore += this->scoreParsimony() * PARSIMONY_SCORE_WEIGHT;

    return totalScore;
}

double Simulation::scoreAccuracy() {
    double accuracyScore{0};
    for (const auto& c : counties) {
        accuracyScore += c->getScore();
    }
    accuracyScore /= counties.size();
    return accuracyScore;
}

double Simulation::scoreSpecificity() { return 0.0; }

double Simulation::scoreParsimony() {

    // Count the number of descriptors which have one or more memebr counties
    int descriptorsCount = count_if(descriptors.cbegin(), descriptors.cend(), [](const auto& d) {
        return !(d->getMemberCountiesIndices().empty());
    });
    
    return 1.0 / (descriptorsCount - 51); // Subtract 51 for fixed-membership descriptors. Consider making this calculated or constant
}

double Simulation::scoreLocality() { return 0.0; }

string Simulation::to_string() {
    stringstream ss;
    ss << "[" << setfill('0') << setw(2) << threadNum << "] Iter: " << iter << ", Temp: " << setprecision(6) << temperature << ", Score: " << score();
    return ss.str();
}

json Simulation::to_json() {
    /*
    {
        "_details": {
            "simulation_details" : 1,
            "score_total" : 1.0,
            ...
        },
        "counties" : [
            "01001" : { // county FIPS (no need to store nation and state - 1 named descriptor each)
                "score" : 1.0,
                "descriptors" : ["$$USA", "$AL", "1", "2"] // list the member descriptors
            },
            ... // each county
        ],
        "descriptors" : [
            "$$USA" : { // nation descriptor
                "demographic" : 0.5, // list all the demographics and values (if nonzero)
                ...
            },
            "$AL" : {...}, // each state
            ...
            "DESCRIPTOR_1" : {...}, // each other descriptor
            ...
        ]
    }
    */

    // Simulation details json
    json simDetailsJson = {
        { "score_total", score() }
    };

    // Counties json
    json countiesJson = {};
    for (const auto& c : counties) {
        auto countyJson = c->to_json();
        countiesJson.update(countyJson);
    }

    // Descriptors json
    json descriptorsJson = {};
    for (const auto& d : descriptors) {
        auto descriptorJson = d->to_json();
        descriptorsJson.update(descriptorJson);
    }
    
    // Combine
    json simJson = {
        { "_details", simDetailsJson },
        { "counties", countiesJson },
        { "descriptors", descriptorsJson }
    };

    return simJson;
}