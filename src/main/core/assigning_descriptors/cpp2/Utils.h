#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <map>
#include <random>
#include <chrono>
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
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

#endif