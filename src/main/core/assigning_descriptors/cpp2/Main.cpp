#include "Constants.h"
#include "Descriptor.h"
#include "County.h"
#include "utils.h"

#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <thread>
#include <mutex>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;

using namespace std;

#define RESOURCES_DIR "../../../resources/"
#define LOGS_DIR "../../../../../logs/"

atomic<bool> stopRequested = false;

struct Simulation {
    uint32_t nationalPopulation;
    array<Descriptor, NUMBER_DESCRIPTORS> descriptors;
    vector<unique_ptr<County>> counties;
    array<string, NUMBER_DEMOGRAPHICS> demographicNames;
    uint32_t tries;
    uint16_t threadNum;

    Simulation() = default;

    // Deep copy constructor
    Simulation(const Simulation& other)
        : nationalPopulation(other.nationalPopulation),
          descriptors(other.descriptors),
          demographicNames(other.demographicNames)
    {
        counties.reserve(other.counties.size());
        for (const auto& c : other.counties) {
            auto newCounty = make_unique<County>(
                c->getName(),
                c->getCountyFIPS(),
                c->getPopulation(),
                c->getDemographics(),
                &descriptors
            );
            counties.push_back(move(newCounty));
        }
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
            auto newCounty = make_unique<County>(
                c->getName(),
                c->getCountyFIPS(),
                c->getPopulation(),
                c->getDemographics(),
                &descriptors
            );
            counties.push_back(move(newCounty));
        }
        return *this;
    }

    // Move semantics
    Simulation(Simulation&&) noexcept = default;
    Simulation& operator=(Simulation&&) noexcept = default;

    void initialize();
    void run();
    double score();
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

string progressBar(double percent, int width) {
    int filled = static_cast<int>(width * percent);
    string res;
    for (int i = 0; i < width; i++) {
        res += (i <= filled ? "#" : "_");
    }
    return res;
}

ThreadSafeLogger logger;

int main() {
    
    // Add console handler for user interrupt
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Error: Coult not set control handler.\n";
        return 1;
    }

    const unsigned int workers = min(max(MIN_THREADS, thread::hardware_concurrency()), MAX_THREADS);
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

                json simJson = sim.formatResults();
                {
                    lock_guard<mutex> lk(bestMutex);
                    if (s > bestScore) {
                        bestScore = s;
                        bestSimJson = simJson;
                        // Write to log file
                        ofstream o(LOGS_DIR "cpplog.json");
                        o << bestSimJson.dump(4);
                        o.close();
                        logger.logLine("[T", w, "] NEW BEST SCORE: ", setprecision(12), bestScore);
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
        cout << "Best simulation saved to " << LOGS_DIR << "best_cpplog.json\n";
    } else {
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
    // Also gather national population
    nationalPopulation = j["population"];

    size_t descriptorsMade = 0;
    // Create fixed-membership descriptors
    // Nation
    descriptors[descriptorsMade] = Descriptor("$$USA", &demographicNames, false);
    ++descriptorsMade;

    // Each State
    for (const string& abbr : statesAbbreviations) {
        descriptors[descriptorsMade] = Descriptor("$" + abbr, &demographicNames, false); // Dollar sign for lexicographic primacy when sorting
        ++descriptorsMade;
    }

    // Create additional unfixed descriptors
    while (descriptorsMade < NUMBER_DESCRIPTORS) {
        descriptors[descriptorsMade] = Descriptor(to_string(descriptorsMade), &demographicNames);
        ++descriptorsMade;
    }

    // Read Counties and assign fixed descriptors
    size_t stateOrder = 1;
    for (const string& state : listDirectories(RESOURCES_DIR)) {
        logger.logLine("Initializing ", state);
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

            auto countyPtr = make_unique<County>(
                j.value("name", string()),
                j.value("FIPS", string()),
                j.value("population", uint32_t()),
                demoArray,
                &descriptors
            );
            countyPtr->addDescriptor(stateOrder);
            counties.emplace_back(move(countyPtr));
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

    struct Change {
        function<void()> undo_fn;
        Change(function<void()> fn) : undo_fn(fn) {}
        void undo() { if (undo_fn) undo_fn(); }
    };

    uint64_t max_iter = 10'000'000, iter = 0;
    Change ch{[](){}};
    double prevScore = 0, newScore = 0;
    while (iter++ < max_iter && tries < MAX_TRIES && !stopRequested) {
        // cout << iter << " ";
        // Chance that a change made is to a descriptor rather than a county
        if (randomChance(0.99)) {
            // cout << "D ";
            // Change a descriptor
            // Choose a descriptor
            size_t d = randomInt(0, NUMBER_DESCRIPTORS);
            // Choose an effect to modify
            size_t e = randomInt(0, NUMBER_DEMOGRAPHICS);
            // Choose an amount to modify by
            double change = randomDouble(-MAX_CHANGE_AMT, MAX_CHANGE_AMT);
            // Make the change
            double prev = descriptors[d].getEffect(e);
            descriptors[d].addEffect(e, change);
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
            ch = Change([&c, d]() mutable {
                c.addOrRemoveDescriptor(d);
            });
        }

        // Evaluate
        newScore = score();
        if (newScore < prevScore) { // If not better, revert
            ch.undo();
            ++tries;
        }
        else { // Keep the change
            prevScore = newScore;
            tries = 0;
        }

        // Print details
        if (iter % 10000 == 0)
            logger.logLine("[T", setfill('0'), setw(2), to_string(threadNum), "]",
            "Iter: ", setw(8), iter, ", ",
            "Acc: ", fixed, setprecision(12), setw(14), newScore * 100, "% ",
            progressBar(newScore, 100));
        // cout << newScore << endl;
    }

    if (stopRequested) {
        logger.logLine("\n[T", setfill('0'), setw(2), to_string(threadNum), "] Simulation interrupted by user.");
    }
    else if (tries >= MAX_TRIES) {
        logger.logLine("\n[T", setfill('0'), setw(2), to_string(threadNum), "] ", to_string(MAX_TRIES), " iterations without improvement. Dropping out.");
    }
    else {
        logger.logLine("\n[T", setfill('0'), setw(2), to_string(threadNum), "] Simulation limit reached.");
    }

}

double Simulation::score() {
    // Loop through counties
    // Use county accuracy * ratio county pop / national pop
    // Sum all values for total accuracy
    return accumulate(
        counties.begin(), counties.end(), 0.0,
        [this](double total, const unique_ptr<County>& c) {
            return total + c->getScore() * (static_cast<double>(c->getPopulation()) / nationalPopulation);
        }
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
        { "accuracy", score()},
        { "counties", allCounties },
        { "descriptors", allDescriptors }
    };

    return simJson;
}