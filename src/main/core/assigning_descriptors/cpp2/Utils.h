#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <windows.h>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;

#include "Constants.h"

inline std::mt19937& rng() {
    static std::mt19937 engine(static_cast<unsigned>(std::time(nullptr)));
    return engine;
}

void flattenJson(
    const json& j,
    std::map<std::string, double>& result,
    const std::string& parentKey = "",
    const std::string& sep = "->"
);

std::vector<std::string> getJsonNestedKeys(const json& j);

size_t countJsonNestedKeys(const json& j);

json freadJson(const std::string& path);

extern std::array<std::string, 51> statesAbbreviations;

extern std::map<std::string, std::string> stateNameToAbbr;

extern std::map<std::string, std::string> stateFIPSToName;

std::vector<std::string> listDirectories(const std::string& path);

std::vector<std::string> listFiles(const std::string& path);

double compareDemographics(const std::array<double, NUMBER_DEMOGRAPHICS>& expected, const std::array<double, NUMBER_DEMOGRAPHICS>& actual, std::string method = "js");

int randomInt(int min, int max);

double randomDouble(double min, double max);

template<typename T, size_t N>
T& randomItem(std::array<T, N>& arr) {
    if (arr.size() == 0) throw std::out_of_range("randomItem(arr): array is empty");
    if (arr.size() == 1) return arr[0];
    return arr[randomInt(0, static_cast<int>(N-1))];
}

template<typename T>
T& randomItem(std::vector<T>& vec) {
    if (vec.empty()) throw std::out_of_range("randomItem(vec): vector is empty");
    if (vec.size() == 1) return vec[0];
    return vec[randomInt(0, static_cast<int>(vec.size() - 1))];
}

bool randomChance(float chance);

template<typename T>
concept Number = std::is_arithmetic_v<T>;

template <Number T>
double std_dev(const std::vector<T>& vec) {
    int n = vec.size();
    if (n == 0) return 0.0;

    double sum = std::accumulate(vec.cbegin(), vec.cend(), 0.0);
    double mean = sum / n;
    double squared_diff_sum = 0.0;
    for (const T& value : vec) {
        squared_diff_sum += std::pow(value - mean, 2);
    }
    double variance = squared_diff_sum / n;
    double std_dev = std::sqrt(variance);
    return std_dev;
}

class ThreadSafeLogger {
private:
    static inline std::mutex logMutex;
public:
    template<typename T>
    ThreadSafeLogger& operator<<(const T& value) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << value;
        return *this;
    }

    ThreadSafeLogger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << manip;
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

std::string progressBar(double percent, int width, bool showPercent = true);

#endif