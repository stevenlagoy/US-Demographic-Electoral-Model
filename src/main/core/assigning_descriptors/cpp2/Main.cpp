#include "Constants.h"
#include "Descriptor.h"
#include "County.h"
#include "utils.h"

#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;

using namespace std;

#define RESOURCES_DIR "../../../resources/"

struct Simulation {
    array<Descriptor, NUMBER_DESCRIPTORS> descriptors;
    vector<County> counties;
    array<string, NUMBER_DEMOGRAPHICS> demographicNames;

    Simulation() : descriptors{}, counties{}, demographicNames{} {}

    void initialize();
    void run();
    void printResults();
};

int main() {
    
    Simulation sim;
    cout << "Initializing..." << endl;
    sim.initialize();
    cout << "Initialization done! Running simulation..." << endl;
    sim.run();
    cout << "Simulation complete! Printing results..." << endl;
    sim.printResults();
    cout << "Main done!" << endl;

    return 0;
}

void Simulation::initialize() {
    counties.clear();

    // Read demographic names
    // Can arbitrarily use nation data for this
    
    json j = freadJson(RESOURCES_DIR "nation.json");
    json demoJson = j["demographics"];
    auto keys = getJsonNestedKeys(demoJson);
    size_t numValues = keys.size();
    if (numValues != NUMBER_DEMOGRAPHICS) {
        stringstream ss;
        ss << "Number of demographics in datafile does not match declared constant: " 
            << numValues << " != " << NUMBER_DEMOGRAPHICS;
        throw runtime_error(ss.str());
    }
    // Read demographics keys into names array
    for (size_t i = 0; i < numValues; ++i) {
        demographicNames[i] = keys[i];
    }

    size_t descriptorsMade = 0;
    // Create fixed-membership descriptors
    // Nation
    descriptors[descriptorsMade] = Descriptor("USA", false);
    ++descriptorsMade;

    // Each State
    for (const string& abbr : statesAbbreviations) {
        descriptors[descriptorsMade] = Descriptor(abbr, false);
        ++descriptorsMade;
    }

    // Create additional unfixed descriptors
    while (descriptorsMade < NUMBER_DESCRIPTORS) {
        descriptors[descriptorsMade] = Descriptor(to_string(descriptorsMade));
        ++descriptorsMade;
    }

    // Read Counties and assign fixed descriptors
    size_t stateOrder = 1;
    for (const string& state : listDirectories(RESOURCES_DIR)) {
        cout << "Initializing " << state << endl;
        string countyDir = RESOURCES_DIR + state + "/counties/";
        for (const string& file : listFiles(countyDir)) {
            // Verify file name: must contain only digits before a dot ("01001.json", "56045.json", ...)
            if (file.find('.') == string::npos || !all_of(file.begin(), file.begin() + file.find('.'), ::isdigit)) continue;

            json j = freadJson(countyDir + "\\" + file);

            array<double, NUMBER_DEMOGRAPHICS> demoArray;
            map<string, double> flat;
            flattenJson(j["demographics"], flat);
            size_t i = 0;
            for (const auto& [k, v] : flat) {
                demoArray[i++] = v;
            }

            counties.emplace_back(
                j.value("name", string()),
                j.value("FIPS", string()),
                j.value("population", uint32_t()),
                demoArray,
                &descriptors
            ).addDescriptor(stateOrder);
        }
        ++stateOrder;
    }

}

void Simulation::run() {

    // Make a change
        // Determine what kind of change
        // To a descriptor?
            // Choose a descriptor
            // Choose an effect to modify
            // Choose an amount to modify by
            // Make the change
        // To a county?
            // Choose a descriptor
            // If county already a member, remove membership
            // If county not already a member, add membership

    // If change is not better, revert it

    // Repeat

}

void Simulation::printResults() {
    // Print to json
    /*
    {
        "counties" : [
            "01001" : { // county FIPS (no need to store nation and state - 1 named descriptor each)
                "descriptors" : ["USA", "AL", "1", "2"], // list the member descriptors
                "accuracy" : 0.65 // accuracy achieved by the model
            },
            ... // each county
        ],
        "descriptors" : [
            "USA" : { // nation descriptor
                "demographic" : 0.5, // list all the demographics and values (if nonzero)
                ...
            },
            "AL" : {...}, // each state
            ...
            "1" : {...}, /// each other descriptor
            ...
        ]
    }
    */
    cout << "Generated " << counties.size() << " counties" << endl;
    for (const auto c : counties) {
        if (c.getStateFIPS() == "01" || c.getStateFIPS() == "10" || c.getStateFIPS() == "11" || c.getStateFIPS() == "12" || c.getStateFIPS() == "50") {
            cout << c.toString() << endl;
        }
    }
    // for (const auto d : descriptors) {
    //     cout << d.toString() << endl;
    // }
}