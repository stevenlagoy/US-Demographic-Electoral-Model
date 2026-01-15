#include "Constants.h"
#include "Descriptor.h"
#include "County.h"
#include "utils.h"

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

atomic<bool> stopRequested = false;

struct Simulation {
    chrono::steady_clock::time_point startTime = chrono::steady_clock::now();
    uint32_t nationalPopulation = 0;
    array<Descriptor, NUMBER_DESCRIPTORS> descriptors;
    vector<unique_ptr<County>> counties;
    array<string, NUMBER_DEMOGRAPHICS> demographicNames;
    uint64_t iter = 0;
    uint32_t tries = 0;
    uint16_t threadNum = 0;
    double temperature = STARTING_TEMPERATURE;

    Simulation()
        : nationalPopulation{0},
          iter{0},
          tries{0},
          threadNum{0},
          temperature{STARTING_TEMPERATURE}
    {
        descriptors = array<Descriptor, NUMBER_DESCRIPTORS>();
        counties = vector<unique_ptr<County>>();
        demographicNames = array<string, NUMBER_DEMOGRAPHICS>();
    }

    ~Simulation() = default;

    // Deep copy constructor
    Simulation(const Simulation& other)
        : nationalPopulation(other.nationalPopulation),
          descriptors(other.descriptors),
          demographicNames(other.demographicNames)
    {
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            auto newCounty = make_unique<County>(*c);
            // Add descriptors
            for (const auto& di : c->getDescriptorIndices()) {
                if (!newCounty->hasDescriptor(di))
                    newCounty->addDescriptor(di);
            }
            counties.push_back(move(newCounty));
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

class ThreadSafeLogger {
private:
    static inline mutex logMutex;
public:
    template<typename T>
    ThreadSafeLogger& operator<<(const T& value) {
        lock_guard<mutex> lock(logMutex);
        cout << value;
        return *this;
    }

    ThreadSafeLogger& operator<<(ostream& (*manip)(ostream&)) {
        lock_guard<mutex> lock(logMutex);
        cout << manip;
        return *this;
    }

    template<typename... Args>
    static void log(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args)); // fold expression to write all args
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << oss.str() << std::flush;
    }

    template<typename... Args>
    static void logLine(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << oss.str() << std::endl;
    }
};

string progressBar(double percent, int width, bool showPercent = true) {
    int filled = static_cast<int>((width - 2) * percent);
    ostringstream res;
    const string startSymbol  = "["; //"▕";
    const string filledSymbol = "#"; //"█";
    const string emptySymbol  = "-"; //"─";
    const string endSymbol    = "]"; //"▏";
    const string lowColor = ESC CSI FG_RED SGR;
    const string midColor = ESC CSI FG_YELLOW SGR;
    const string highColor = ESC CSI FG_GREEN SGR;
    const string percentLabelColor = ESC CSI FG_BRIGHT_WHITE SGR;
    
    // Determine location to show percent
    // Try to avoid covering the current value
    // If not possible, put percentage after the bar
    // Do not exceed width, even with percentage after the bar
    // Favor the middle, then the right side, then the left side
    // Center between the percent and the ends of the bar
    int percentLabelWidth = min(width / 4, 12);
    int wholePartWidth = max(1, (int)(log10(percent)));
    int decimalPartWidth = percentLabelWidth - wholePartWidth - 2; // Subtract 2 for the decimal and percent sign
    int percentLabelCenter = percent < 0.5 ? (width - filled) / 2 + filled : filled / 2;
    // Snap to quarters
    // .0-.375 = .25  .375-.625 = .5  .625-1.0 = .75
    if (percentLabelCenter < (3 * width / 8)) {
        percentLabelCenter = (1 * width / 4);
    }
    else if (percentLabelCenter < (5 * width / 8)) {
        percentLabelCenter = (2 * width / 4);
    }
    else {
        percentLabelCenter = (3 * width / 4);
    }
    int percentLabelAround = percentLabelWidth / 2;
    int percentLabelStart = percentLabelCenter - percentLabelAround;
    int percentLabelEnd = percentLabelStart + percentLabelWidth;
    if (percentLabelStart < 1 || percentLabelEnd > width - 1) {
        // Place the percent label after the progress bar
        width -= percentLabelWidth;
        percentLabelStart = width + 1;
    }

    res << startSymbol;
    for (int i = 1; i < (width - 1); i++) {
        if (i == percentLabelStart && showPercent) {
            res << percentLabelColor << fixed << setprecision(decimalPartWidth) << percent * 100 << "%" << RESET;
        }
        if (i >= percentLabelStart && i <= percentLabelEnd && showPercent) continue;
        string color = i > 2 * (width / 3) ? highColor : i > (width / 3) ? midColor : lowColor;
        res << (i <= filled ? (color + filledSymbol + RESET) : emptySymbol);
    }
    res << endSymbol;
    return res.str();
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
        cerr << "Error: Coult not set control handler.\n";
        return 1;
    }

    const unsigned int workers = min(max(MIN_THREADS, thread::hardware_concurrency()), isDebug ? MAX_DEBUG_THREADS : MAX_THREADS);
    logger << "Starting " << workers << " parallel simulations..." << endl;

    mutex bestMutex;
    double bestScore = -1.0;
    json bestSimJson = json::object();

    vector<thread> threads;
    threads.reserve(workers);

    // Build a base simulation object to avoid initializing counties multiple times
    Simulation baseSim;
    logger.logLine("Initializing base model...\n");
    baseSim.initialize();

    for (unsigned int w = 0; w < workers; ++w) {
        threads.emplace_back([w, &bestMutex, &bestSimJson, &bestScore, &baseSim]() {
            try {
                Simulation sim = baseSim;
                sim.threadNum = w;
                // logger.logLine("[T", w, "] Initializing...\n");
                // sim.initialize();
                logger.logLine("[T", w, "] Running...");
                sim.run();
                double s = sim.score();

                {
                    lock_guard<mutex> lk(bestMutex);
                    if (s > bestScore) {
                        bestScore = s;
                        bestSimJson = sim.formatResults();
                        // Write to log file
                        ofstream o(LOGS_DIR "cpplog.json");
                        o << bestSimJson.dump(4);
                        o.close();
                        logger.logLine("[T", w, "] BEST SCORE: ", setprecision(12), bestScore);
                    }
                }
            }
            catch (const exception& ex) {
                cerr << "[T" << w << "] Exception: " << ex.what() << "\n";
            }
            catch (...) {
                cerr << "[T" << w << "] Unknown exception\n";
            }
        });
    }

    // Join
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    if (bestScore >= 0.0) {
        cout << "Best score across workers: " << setprecision(12) << bestScore << "\n";
        // Writing is done in the main loop whenever a best score is found
        cout << "Best simulation saved to " << LOGS_DIR << "cpplog.json\n";
    }
    else {
        cout << "No successful simulations completed.\n";
    }

    return 0;
    
    // Non-parallel version
    // Simulation sim;
    // cout << "Initializing..." << endl;
    // sim.initialize();
    // cout << "Initialization done! Running simulation..." << endl;
    // sim.run();
    // cout << "Simulation complete! Printing results..." << endl;
    // sim.printResults();
    // cout << "Main done!" << endl;

    // return 0;
}

void Simulation::initialize() {
    counties.clear();
    this->startTime = chrono::steady_clock::now();

    // Read demographic names
    // Can use nation data for this, as all files have same demographic keys
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
    // Also gather national population
    nationalPopulation = j["population"];

    size_t descriptorsMade = 0;
    // Create fixed-membership descriptors
    // Nation
    descriptors[descriptorsMade] = Descriptor("$$USA", &demographicNames, false);
    ++descriptorsMade;

    // Each State
    map<string, size_t> stateToDescriptor;
    for (const string& abbr : statesAbbreviations) {
        descriptors[descriptorsMade] = Descriptor("$" + abbr, &demographicNames, false); // Dollar sign for lexicographic primacy when sorting
        stateToDescriptor[abbr] = descriptorsMade; // Map abbreviation to descriptor index
        ++descriptorsMade;
    }

    // Create additional unfixed descriptors
    while (descriptorsMade < NUMBER_DESCRIPTORS) {
        string name = to_string(descriptorsMade);
        name.append(3 - name.length(), '0'); // Pad forwards with zeroes
        descriptors[descriptorsMade] = Descriptor(name, &demographicNames);
        ++descriptorsMade;
    }

    // Read county adjacencies
    string adjacenciesFile = RESOURCES_DIR "adjacencies.json";
    json adjacenciesJson = freadJson(adjacenciesFile);
    map<string, vector<string>> adjacencies{};
    for (const auto& i : adjacenciesJson.items()) {
        vector<string> neighbors{};
        for (const auto& v : i.value()) {
            neighbors.emplace_back(v);
        }
        adjacencies[i.key()] = neighbors;
    }

    // Read Counties and assign fixed descriptors
    for (const string& state : listDirectories(RESOURCES_DIR)) {
        // logger.logLine("Initializing ", state);
        string countyDir = RESOURCES_DIR + state + "/counties/";

        // Convert state name to abbreviation
        auto nameToAbbrIt = stateNameToAbbr.find(state);
        if (nameToAbbrIt == stateNameToAbbr.end()) {
            logger.logLine("Warning: Unknown state name: ", state);
            continue;
        }
        string stateAbbr = nameToAbbrIt->second;

        // Find descriptor index
        auto stateDescIt = stateToDescriptor.find(stateAbbr);
        if (stateDescIt == stateToDescriptor.end()) {
            logger.logLine("Warning: No descriptor found for state abbreviation: ", stateAbbr);
            continue;
        }
        size_t stateDescriptorIndex = stateDescIt->second;

        for (const string& file : listFiles(countyDir)) {
            // Verify file name: must contain only digits before a dot ("01001.json", "56045.json", ...)
            if (file.find('.') == string::npos || !all_of(file.begin(), file.begin() + file.find('.'), ::isdigit)) continue;

            json j = freadJson(countyDir + "\\" + file);

            array<double, NUMBER_DEMOGRAPHICS> demoArray;
            map<string, double> flat;
            flattenJson(j["demographics"], flat);
            if (flat.size() != NUMBER_DEMOGRAPHICS) {
                logger.logLine(file + " Flat size: " + to_string(flat.size()) + " NUMBER_DEMOGRAPHICS " + to_string(NUMBER_DEMOGRAPHICS));
            }
            assert(flat.size() <= NUMBER_DEMOGRAPHICS);
            size_t i = 0;
            for (const auto& [k, v] : flat) {
                demoArray[i++] = v;
            }

            auto countyPtr = make_unique<County>(
                j.value("name", string()),
                j.value("FIPS", string()),
                j.value("population", uint32_t()),
                adjacencies[j.value("FIPS", string())],
                demoArray,
                &descriptors
            );
            // Add nation (index 0)
            // countyPtr->addDescriptor(0); // This is done in the county's constructor
            // Add state descriptor
            // logger.logLine("Assigning descriptor ", descriptors[stateDescriptorIndex].getName(), " (", stateDescriptorIndex, ") to county ", countyPtr->getName());
            countyPtr->addDescriptor(stateDescriptorIndex);
            descriptors[stateDescriptorIndex].addMemberCounty(&(*countyPtr)); // Add to state descriptor
            descriptors[0].addMemberCounty(&(*countyPtr)); // Add to national descriptor

            counties.emplace_back(move(countyPtr));
        }
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

    struct Change {
        function<void()> undo_fn;
        Change(function<void()> fn) : undo_fn(fn) {}
        void undo() { if (undo_fn) undo_fn(); }
    };

    Change ch{[](){}};
    double prevScore = 0, newScore = 0;
    while (iter++ < MAX_ITERATIONS && tries < MAX_TRIES && !stopRequested) {
        temperature = clamp(temperature - TEMPERATURE_STEP, 0.0, 1.0);
        // cout << iter << " ";
        // Chance that a change made is to a descriptor rather than a county
        if (randomChance(CHANGE_DESCRIPTOR_CHANCE)) {
            // cout << "D ";
            // Change a descriptor
            // Choose a descriptor
            size_t d = randomInt(0, NUMBER_DESCRIPTORS);
            assert(d < NUMBER_DESCRIPTORS);
            // Choose an effect to modify
            size_t e = randomInt(0, NUMBER_DEMOGRAPHICS);
            assert(e < NUMBER_DEMOGRAPHICS);
            // Choose an amount to modify by
            double change = randomDouble(-MAX_CHANGE_AMT, MAX_CHANGE_AMT);
            // Make the change
            double prev = descriptors[d].getEffect(e);
            descriptors[d].addEffect(e, change);
            // Propegate change to member counties
            // for (const County& c : counties) {
            //     if (c.hasDescriptor(d)) c.recalculate();
            // }
            ch = Change([this, d, e, prev]() mutable {
                descriptors[d].setEffect(e, prev);
            });
        }
        else {
            // cout << "C ";
            // Change a county
            // Choose a county
            County& c = *counties[randomInt(0, counties.size())];
            // Choose a membership-modifiable descriptor
            size_t d;
            do {
                d = randomInt(0, NUMBER_DESCRIPTORS);
            } while (!descriptors[d].isMembershipModifiable());
            // If county is a member, remove membership
            // If county not a member, add membership
            c.addOrRemoveDescriptor(d);
            descriptors[d].addOrRemoveMemberCounty(&c);
            ch = Change([&c, d, this]() mutable {
                c.addOrRemoveDescriptor(d);
                descriptors[d].addOrRemoveMemberCounty(&c);
            });
        }

        // Evaluate
        newScore = score();
        if (newScore < prevScore) { // The change made the simulation worse
            // Temperature check
            if (randomChance(this->temperature)) {
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

        // Print details
        if (iter % PRINT_TSTATUS_EVERY == 0)
            this->logStatus();
        // cout << newScore << endl;
    }

    if (stopRequested) {
        logger.logLine("[T", setfill('0'), setw(2), to_string(threadNum), "] Simulation interrupted by user.");
    }
    else if (tries >= MAX_TRIES) {
        logger.logLine("[T", setfill('0'), setw(2), to_string(threadNum), "] ", to_string(MAX_TRIES), " iterations without improvement. Dropping out.");
    }
    else {
        logger.logLine("[T", setfill('0'), setw(2), to_string(threadNum), "] Simulation limit reached.");
    }
}

double Simulation::score() {
    // Loop through counties
    // Use county accuracy * ratio county pop / national pop
    // Sum all values for total accuracy

    double accuracy, specificity, parsimony, locality;
    accuracy    = this->scoreAccuracy();
    specificity = this->scoreSpecificity();
    parsimony   = this->scoreParsimony();
    locality    = this->scoreLocality();

    accuracy    *= ACCURACY_SCORE_WEIGHT;
    specificity *= SPECIFICITY_SCORE_WEIGHT;
    parsimony   *= PARSIMONY_SCORE_WEIGHT;
    locality    *= LOCALITY_SCORE_WEIGHT;

    double totalScore = accuracy + specificity + parsimony + locality;

    return totalScore;
}

double Simulation::scoreAccuracy() {
    // Accuracy is the sum of each county's accuracy weighted by its population
    // 1.0 -> perfectly accurate; 0.0 -> completely innacurate or empty
    double accuracy = accumulate(
        counties.begin(), counties.end(), 0.0,
        [this](double total, const unique_ptr<County>& c) {
            return total + c->getScore() * (static_cast<double>(c->getPopulation()) / nationalPopulation);
        }
    );
    return accuracy;
}

double Simulation::scoreSpecificity() {
    // Specificity is the sum of squares of each descriptor's effects
    // >=1.0 -> Every descriptor has 100% effects across the board; 0% -> Every descriptor has no effects
    double specificity = accumulate(
        descriptors.cbegin(), descriptors.cend(), 0.0,
        [this](double total, const Descriptor& d) {
            return total + d.getSpecificityScore();
        }
    ) / descriptors.size();
    return min(specificity, 1.0);
}

double Simulation::scoreParsimony() {
    // Parsimony is inverse to the number of used descriptors
    // 1.0 -> no descriptors were used; 0.0 -> all possible descriptors were used
    size_t numEffectualDescriptors = std::count_if(descriptors.cbegin(), descriptors.cend(), [](const Descriptor& d){ return d.hasAnyEffect(); });
    double parsimony = 1.0 - numEffectualDescriptors / NUMBER_DESCRIPTORS;
    return parsimony;
}

double Simulation::scoreLocality() {
    // Locality compares actual to expected number of member neighbors for member counties
    // >=1.0 -> Every descriptor's members each have the expected number of neighbors; 0.0 -> No descriptor has any members which are neighbors
    double locality = accumulate(descriptors.cbegin(), descriptors.cend(), 0.0, [](double total, const Descriptor& d) { return total + d.getLocalityScore(); }) / NUMBER_DESCRIPTORS;
    return locality;
}

void Simulation::logStatus() {
    double currScore = score();
    int lineNum = threadNum + 1;
    logger.logLine(
        ESC, CSI, to_string(lineNum), SEP, "1", CUP,
        CLEAR_LINE,
        ESC, CSI, FG_BRIGHT_WHITE, SGR,
        " [T", setfill('0'), setw(2), to_string(threadNum), "] ",
        "Iter=", setw(7), iter, " ",
        "Temp=", setw(8), to_string(temperature), " ",
        "Acc:", progressBar(scoreAccuracy(), 20), " ",
        "Spc:", progressBar(scoreSpecificity(), 20), " ",
        "Par:", progressBar(scoreParsimony(), 20), " ",
        "Loc:", progressBar(scoreLocality(), 20), " ",
        "TOTAL:", progressBar(currScore, 50),
        RESET
    );
}

json Simulation::formatResults() {
    // Convert to json
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

    // Simulation details json
    json simDetails = {
        { "score_total", score() },
        { "score_accuracy", scoreAccuracy() },
        { "score_specificity", scoreSpecificity() },
        { "score_parsimony", scoreParsimony() },
        { "score_locality", scoreLocality() },
        { "sim_runtime", chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - this->startTime).count() },
        { "number_counties", this->counties.size() },
        { "number_descriptors", NUMBER_DESCRIPTORS },
        { "number_demographics", NUMBER_DEMOGRAPHICS },
        { "starting_temperature", STARTING_TEMPERATURE },
        { "temperature_step", TEMPERATURE_STEP },
        { "max_change_amount", MAX_CHANGE_AMT },
        { "change_descriptor_chance", CHANGE_DESCRIPTOR_CHANCE },
        { "change_county_chance", CHANGE_COUNTY_CHANCE }
    };

    // Counties json
    json allCounties = json::object();
    for (const auto& c : counties) {
        auto countyJson = c->toJson();
        allCounties.update(countyJson);
    }

    // Descriptors json
    json allDescriptors = json::object();
    for (const auto& d : descriptors) {
        auto descJson = d.toJson();
        allDescriptors.update(descJson);
    }

    // Combine for simulation json
    json simJson = {
        { "details", simDetails },
        { "counties", allCounties },
        { "descriptors", allDescriptors }
    };

    return simJson;
}