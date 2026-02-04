#include "Constants.h"
#include "County.h"
#include "Descriptor.h"
#include "Utils.h"

#include <chrono>
#include <cmath>
#include <vector>
#include <iostream>
#include <array>
#include <random>
#include <limits>
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

// Eigen please don't segfault
#define EIGEN_DONT_ALIGN_STATICALLY
#define EIGEN_DONT_VECTORIZE
#include "../../../../lib/Eigen/Dense"
using Eigen::MatrixXd;

using namespace std;

#define RESOURCES_DIR "../../../resources/2020/"
#define LOGS_DIR "../../../../../logs/"
#define NATION_FILE RESOURCES_DIR "nation.json"
#define LOG_FILE LOGS_DIR "cpplog.json"

struct Simulation {

    uint32_t nationalPopulation = 0;
    uint16_t threadNum = 0;
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

    double score();

    string to_string();
    json to_json();
};

struct Cluster {
    Eigen::VectorXd center;
    double popSum = 0.0;
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
bool isDebug = false;

int main(int argc, char* argv[]) {

    cout << HIDE_CURSOR;

    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [-debug]";
        return -1;
    }
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

    baseSim.run();

    json simJson = baseSim.to_json();

    ofstream ofs(LOG_FILE);
    ofs << simJson.dump(4);
    ofs.close();

    logger.logLine("Done!");

    return 0;
}

void Simulation::initialize() {
    startTime = chrono::steady_clock::now();
    auto initStart = chrono::system_clock::now();
    time_t start_c = chrono::system_clock::to_time_t(initStart);
    ostringstream oss1;
    oss1 << put_time(localtime(&start_c), "%T");
    logger.logLine("Beginning initialization at " + oss1.str());
    
    createCounties();
    createDescriptors();
    this->_INITIALIZED = true;

    auto initEnd = chrono::system_clock::now();
    time_t end_c = chrono::system_clock::to_time_t(initEnd);
    auto msElapsed = chrono::duration_cast<chrono::milliseconds>(initEnd - initStart).count();
    ostringstream oss2;
    oss2 << put_time(localtime(&end_c), "%T");
    logger.logLine("Successfully initialized at " + oss2.str() + " (" + ::to_string(msElapsed) + "ms elapsed)");
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
    counties.reserve(NUMBER_COUNTIES); // NUMBER_COUNTIES is approximate - allow implementation to resize if needed
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

            uint32_t pop = j["population"];
            nationalPopulation += pop;

            counties.emplace_back(make_unique<County>(
                j["name"],
                j["FIPS"],
                countiesMade,
                pop,
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
    for (const auto& c : counties) {
        c->addDescriptor(descriptorsMade);
    }
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
            if (c->getStateFIPS() == FIPS) {
                memberCountiesIndices.push_back(c->getIndex());
            }
        }
        descriptors.emplace_back(make_unique<Descriptor>(
            "$" + abbr,
            descriptorsMade,
            &(this->counties),
            memberCountiesIndices,
            false
        ));
        for (const auto& c : counties) {
            if (c->getStateFIPS() == FIPS) {
                // Must do this after creating the descriptor
                c->addDescriptor(descriptorsMade);
            }
        }

        descriptorsMade++;
    }
}

void weightedKMeans(vector<CountyPoint>& pts, int K, int maxIter = 50) {
    int N = pts.size();
    int dim = pts[0].z.size();

    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, N-1);

    vector<Cluster> clusters(K);
    for (int k = 0; k < K; ++k)
        clusters[k].center = pts[dis(gen)].z;

    for (int iter = 0; iter < maxIter; ++iter) {
        for (auto& p : pts) {
            double bestDist = std::numeric_limits<double>::max();
            int bestK = 0;
            for (int k = 0; k < K; ++k) {
                double dist = (p.z - clusters[k].center).squaredNorm();
                if (dist < bestDist) {
                    bestDist = dist;
                    bestK = k;
                }
            }
            p.cluster = bestK;
        }

        // Update
        for (auto& c : clusters) {
            c.center.setZero(dim);
            c.popSum = 0.0;
        }

        for (const auto& p : pts) {
            clusters[p.cluster].center += p.population * p.z;
            clusters[p.cluster].popSum += p.population;
        }

        for (auto& c : clusters) {
            if (c.popSum > 0) c.center /= c.popSum;
        }
    }
}

void Simulation::run() {

    if (!_INITIALIZED) return;

    // min[d_k, c(i)] ( 1/P * ∑[i] ( p_i * ||r_i - d_(c(i))|| ^ 2 ) )

    // Build residual matrix: NUMBER_DEMOGRAPHICS x counties.size()
    auto residual = make_unique<vector<array<double, NUMBER_DEMOGRAPHICS>>>();
    residual->reserve(counties.size());
    for (size_t i = 0; i < counties.size(); ++i) {
        residual->emplace_back(counties[i]->getMissingDemographics());
    }

    // Create covariance matrix: NUMBER_DEMOGRAPHICS x NUMBER_DEMOGRAPHICS
    // Covariance is symmetric positive semidefinite
    auto covariance = make_unique<array<array<double, NUMBER_DEMOGRAPHICS>, NUMBER_DEMOGRAPHICS>>();
    for (auto& row : (*covariance)) row.fill(0.0);
    for (size_t i = 0; i < counties.size(); ++i) {
        uint32_t population = counties[i]->getPopulation();
        auto cov = vectorToSquareMatrix((*residual)[i]);
        for (size_t row = 0; row < cov.size(); ++row) {
            for (size_t col = 0; col < cov[row].size(); ++col) {
                (*covariance)[row][col] += (cov[row][col] * population) / nationalPopulation;
            }
        }
    }
    // Check for invalid values
    for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
        for (size_t j = 0; j < NUMBER_DEMOGRAPHICS; ++j) {
            if (!isfinite((*covariance)[i][j])) {
                logger.logLine("Covariance matrix contains NaN or Inf at (", i, ",", j, ")");
                return; // Early out
            }
        }
    }
    if (NUMBER_DEMOGRAPHICS == 0 || demographicNames.size() == 0) {
        logger.logLine("NUMBER_DEMOGRAPHICS = " + ::to_string(NUMBER_DEMOGRAPHICS) + ", demographicNames.size() == " + ::to_string(demographicNames.size()));
        return;
    }
    if (nationalPopulation == 0) {
        logger.logLine("nationalPopulation = " + ::to_string(nationalPopulation));
        return;
    }

    if (isDebug) {
        ofstream ofs(LOGS_DIR "covariance.txt");
        for (const auto& row : (*covariance)) {
            for (const auto& col : row) {
                ofs << scientific << setprecision(3) << showpoint << col << " ";
            }
            ofs << "\n";
        }
        ofs.close();
    }

    // Eigen Decomposition with PCA
    // C_v_j = λ_j * v_j

    MatrixXd eigen = toEigen(*covariance);
    PCAResult result = computePCA(eigen);
    int k = chooseComponentCount(result.eigenvalues);
    logger.logLine("Number of PCA components kept: " + ::to_string(k));

    // Precompute PCA projection matrix (NUMBER_DESCRIPTORS x k)
    Eigen::MatrixXd Vk = result.eigenvectors.leftCols(k);

    // Build PCA-space county points
    vector<CountyPoint> points;
    points.reserve(counties.size());

    for (size_t i = 0; i < counties.size(); ++i) {
        Eigen::VectorXd r(NUMBER_DEMOGRAPHICS);
        const auto& resArr = (*residual)[i];
        for (size_t d = 0; d < NUMBER_DEMOGRAPHICS; ++d)
            r(d) = resArr[d];
        
        Eigen::VectorXd z = Vk.transpose() * r;

        points.push_back({ z, (double)counties[i]->getPopulation(), -1 });
    }

    weightedKMeans(points, k);

    // Turn cluster centers back into demographic space
    for (int c = 0; c < k; ++c) {
        logger.logLine("Clustering #", ::to_string(c));
        Eigen::VectorXd center = Eigen::VectorXd::Zero(k);
        double popSum = 0;

        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].cluster == c) {
                center += points[i].population * points[i].z;
                popSum += points[i].population;
            }
        }
        if (popSum == 0) continue;
        center /= popSum;

        // Back-projection to original demographics
        Eigen::VectorXd demoVec = Vk * center;
        
        array<double, NUMBER_DEMOGRAPHICS> demoArr{};
        for (size_t d = 0; d < NUMBER_DEMOGRAPHICS; ++d) demoArr[d] = demoVec(d);

        // Create a descriptor with no members
        size_t idx = descriptors.size();
        stringstream ss;
        ss << "DESC_" << std::setfill('0') << std::right << std::setw(3) << ::to_string(idx);
        descriptors.emplace_back(make_unique<Descriptor>(
            ss.str(),
            idx,
            &counties,
            vector<size_t>{},
            true
        ));

        // Assign the counties in this cluster
        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].cluster == c) {
                descriptors.back()->addMemberCounty(i);
                counties[i]->addDescriptor(idx);
            }
        }
    }
}

double Simulation::score() {

    double accuracyScore{0};
    for (const auto& c : counties) {
        accuracyScore += c->getScore();
    }
    accuracyScore /= counties.size();
    return accuracyScore;

}

string Simulation::to_string() {
    stringstream ss;
    ss << "[" << setfill('0') << setw(2) << threadNum << "] " << score();
    return ss.str();
}

json demographicsJson(const Simulation& sim, const std::array<double, NUMBER_DEMOGRAPHICS>& demographics, bool subtract_from_national = true) {

    if (sim.demographicNames.size() != demographics.size()) {
        throw runtime_error("The demographics are missized");
    }

    array<double, NUMBER_DEMOGRAPHICS> nationalDemographics;
    for (const auto& d : sim.descriptors) {
        if (d->getName() == "$$USA") {
            nationalDemographics = d->getDemographics();
            break;
        }
    }
    if (nationalDemographics.empty()) {
        throw runtime_error("Could not find the national descriptor or demographics.");
    }

    vector<pair<string, double>> sortedDemographics;
    for (size_t i = 0; i < demographics.size(); ++i) {
        double demoVal = demographics[i] - (subtract_from_national ? nationalDemographics[i] : 0);
        if (demoVal >= IMPACTFUL_DEMOGRAPHIC_BOUNDARY || demoVal <= -IMPACTFUL_DEMOGRAPHIC_BOUNDARY)
            sortedDemographics.emplace_back(sim.demographicNames[i], demoVal);
    }
    sort(sortedDemographics.begin(), sortedDemographics.end(), [](const auto& a, const auto& b) { return abs(a.second) > abs(b.second); });
    
    json demographicsJson;
    for (const auto& [name, value] : sortedDemographics) {
        demographicsJson.push_back({ {"name", name}, {"value", value} });
    }

    return demographicsJson;
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
    json countiesJson = json::object();
    for (const auto& c : counties) {
        auto countyJson = c->to_json();
        countiesJson[c->getFIPS()] = countyJson;
    }

    // Descriptors json
    json descriptorsJson = json::object();
    for (const auto& d : descriptors) {
        auto descriptorJson = d->to_json();
        descriptorJson["demographics"] = demographicsJson(*this, d->getDemographics());
        descriptorsJson[d->getName()] = descriptorJson;
    }
    // Add the national demographics
    descriptorsJson["$$USA"]["demographics"] = demographicsJson(*this, descriptors[0]->getDemographics(), false);
    
    // Combine
    json simJson = {
        { "_details", simDetailsJson },
        { "counties", countiesJson },
        { "descriptors", descriptorsJson }
    };

    return simJson;
}