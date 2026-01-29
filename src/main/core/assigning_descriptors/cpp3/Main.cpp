

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

struct Simulation {
    uint32_t nationalPopulation = 0;
    uint64_t iter = 0;
    uint32_t tries = 0;
    uint16_t threadNum = 0;
    double temperature = STARTING_TEMPERATURE;
    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    array<Descriptor, NUMBER_DESCRIPTORS> descriptors;
    vector<unique_ptr<County>> counties;
    unordered_map<string, County*> fipsToCounty;
    array<string, NUMBER_DEMOGRAPHICS> demographicNames;
    bool _INITIALIZED = false;

    Simulation() = default;
    ~Simulation() = default;

    // Copy constructor
    Simulation(const Simulation& other)
        : nationalPopulation(other.nationalPopulation),
          descriptors(other.descriptors),
          demographicNames(other.demographicNames)
    {
        for (auto& d : descriptors)
            d.clearMemberCounties();
        
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            auto newCounty = make_unique<County>(*c);
            counties.push_back(move(newCounty));
        }

        for (const auto& c : counties) {
            for (size_t di : c->getDescriptorIndices()) {
                descriptors[di].addMemberCounty(c.get());
            }
            c->setDescriptorsRef(&descriptors);
        }

        assert(this->descriptors.size() == other.descriptors.size() && this->descriptors.size() == NUMBER_DESCRIPTORS);
        assert(this->demographicNames.size() == other.demographicNames.size() && this->demographicNames.size() == NUMBER_DEMOGRAPHICS);
        assert(this->counties.size() == other.counties.size());
    }

    // Deep copy assignment
    Simulation& operator=(const Simulation& other) {
        if (this == &other) return *this;
        nationalPopulation = other.nationalPopulation;
        descriptors = other.descriptors;
        demographicNames = other.demographicNames;
        counties.clear();
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            auto newCounty = make_unique<County>(*c);
            counties.push_back(move(newCounty));
        }
        assert(this->descriptors.size() == other.descriptors.size() && this->descriptors.size() == NUMBER_DESCRIPTORS);
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
    double scoreAccuracy();
    double scoreSpecificity();
    double scoreParsimony();
    double scoreLocality();
    void logStatus();
    json formatResults();
};

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
    logger << "Initializing base model..." << endl;
    baseSim.initialize();

    // Clone sim into different threads / workers
    const uint8_t workers = clamp(thread::hardware_concurrency(), MIN_THREADS, is_debug ? MAX_DEBUG_THREADS : MAX_THREADS);
    array<thread, workers> threads;
    logger << "Starting " << workers << " parallel simulations..." << endl;
    for (uint8_t w = 0; w < workers; ++w) {
        
    }

    // Run the simulations on their threads
    
    // Join threads
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // Print best simulation

    return 0;
}

void Simulation::initialize() {
    startTime = chrono::steady_clock::now();
    
    createCounties();
    createDescriptors();

    // Create demographics
    // Use nation data, all county files have same demographic keys
    // json j = freadJson(RESOURCES_DIR "nation.json");
    // json demoJson = j["demographics"];
    // auto keys = getJsonNestedKeys(demoJson);
    // size_t numValues = keys.size();
    // if (numValues != NUMBER_DEMOGRAPHICS) {
    //     stringstream ss;
    //     ss << "Number of demographics in datafile does not match declared constant: "
    //         << numValues << " != " << NUMBER_DEMOGRAPHICS;
    //     throw runtime_error(ss.str());
    // }
    // // Read demographics keys into names array
    // for (size_t i = 0; i < numValues; ++i) {
    //     demographicNames[i] = keys[i];
    // }
    // // Also gather national population
    // nationalPopulation = j["population"];

    // // Create descriptors
    // size_t descriptorMade{0};
    // // Create fixed-membership descriptors
    // // Nation
    // descriptors[descriptorsMade] = Descriptor();
    // ++descriptorsMade;

    // // Create counties
    // counties.clear();

    // for (const string& state : listDirectories(RESOURCES_DIR)) {
    //     string countyDir = RESOURCES_DIR + state + "/counties/";

    //     // Convert state name to abbreviation
    //     auto nameToAbbrIt = stateNameToAbbr.find(state);
    //     if (nameToAbbrIt == stateNameToAbbr.end()) {
    //         logger.logLine("Warning: Unknown state name, \"", state, "\"");
    //         continue;
    //     }
    //     string stateAbbr = nameToAbbrIt->second;

    //     const vector<string> countiesNames;
    //     const vector<string> countiesFIPS;
    //     const vector<uint32_t> countiesPopulations;
    //     const vector<unordered_map<string, double>> countiesDemographics;
    //     const vector<vector<string>> countiesNeighborsFIPS;

    //     for (const string& fileName : listFiles(countyDir)) {
    //         // Verify file name: contains only digits before ".json"
    //         if (fileName.find('.') == string::npos || !all_of(file.begin(), file.begin() + file.find('.'), ::isdigit)) continue;

    //         json j = freadJson(countyFir + "\\" + file);

    //         unordered_map<string, double> flatDemographics;
    //         flattenJson(j["demographics"], flatDemographics);
    //         if (flat.size() != NUMBER_DEMOGRAPHICS) {
    //             logger.logLine(file + " Flat size: " + to_string(flat.size()) + " != " + to_string(NUMBER_DEMOGRAPHICS));
    //         }

    //         vector<string> neighborsFIPS;
    //         for (const string& neighborFIPS : j["neighbors"]) {
    //             neighborsFIPS.emplace_back(neighborFIPS);
    //         }

    //         countiesNames.emplace_back(j["name"]);
    //         countiesFIPS.emplace_back(j["FIPS"]);
    //         countiesPopulations.emplace_back(j["population"]);
    //         countiesDemographics.emplace_back(flatDemographics);
    //         countiesNeighborsFIPS.emplace_back(neighborsFIPS);
    //     }

    //     // Check that the vectors have the same length
    //     assert(countiesNames.size() == countiesFIPS.size()
    //         && countiesFIPS.size() == countiesPopulations.size()
    //         && countiesPopulations.size() == countiesDemographics.size()
    //         && countiesDemographics.size() == countiesNeighborsFIPS.size()
    //     );

    //     for (size_t i = 0; i < countiesNames.size(); ++i) {
    //         counties.emplace_back(make_unique<County>(
    //             countiesNames[i],
    //             countiesFIPS[i],
    //             countiesPopulations[i],
    //             countiesDemographics[i],
    //             countiesNeighborFIPS[i]
    //         ));
    //     }
    // }

}

void Simulation::createCounties() {
    counties.clear();

    for (const string& state : listDirectories(RESOURCES_DIR)) {
        string countyDir = RESOURCES_DIR + state + "/counties/";
        for (const string& fileName : listFiles(countyDir)) {
            // Verify file name: contains only digits before ".json"
            if (fileName.find('.') == string::npos || !all_of(file.begin(), file.begin() + file.find('.'), ::isdigit)) continue;

            json j = freadJson(countyDir + "\\" + file);

            counties.emplace_back(make_unique<County>());
        }
    }
}

void Simulation::createDescriptors() {

}

void Simulation::run() {}

double Simulation::score() { return 0.0; }

double Simulation::scoreAccuracy() { return 0.0; }

double Simulation::scoreSpecificity() { return 0.0; }

double Simulation::scoreParsimony() { return 0.0; }

double Simulation::scoreLocality() { return 0.0; }

void Simulation::logStatus() {}

json Simulation::formatResults() {
    return json{};
}