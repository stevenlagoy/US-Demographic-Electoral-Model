#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
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

std::vector<std::string> listDirectories(const std::string& path);

std::vector<std::string> listFiles(const std::string& path);

double compareDemographics(const std::array<double, NUMBER_DEMOGRAPHICS>& expected, const std::array<double, NUMBER_DEMOGRAPHICS>& actual, std::string method = "js");

int randomInt(int min, int max);

double randomDouble(double min, double max);

template<typename T, size_t N>
T& randomItem(std::array<T, N>& arr) {
    return arr[randomInt(0, static_cast<int>(N-1))];
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

#endif