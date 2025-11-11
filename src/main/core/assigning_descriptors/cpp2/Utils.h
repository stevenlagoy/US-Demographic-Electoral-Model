#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <map>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../../../../lib/json.hpp"
using json = nlohmann::json;

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

#endif
