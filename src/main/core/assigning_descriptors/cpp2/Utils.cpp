#include "utils.h"

void flattenJson(
    const json& j,
    std::map<std::string, double>& result,
    const std::string& parentKey,
    const std::string& sep
) {
    for (auto& [k, v] : j.items()) {
        std::string newKey = parentKey.empty() ? k : parentKey + sep + k;

        if (v.is_object()) {
            flattenJson(v, result, newKey, sep);
        } else if (v.is_number_float() || v.is_number_integer()) {
            result[newKey] = v.get<double>();
        } else {
            throw std::runtime_error("Unsupported type for key " + newKey);
        }
    }
}

std::vector<std::string> getJsonNestedKeys(const json& j) {
    std::vector<std::string> result{};
    std::map<std::string, double> flat;
    flattenJson(j, flat);
    for (auto& [k, v] : flat) {
        result.push_back(k);
    }
    return result;
}

size_t countJsonNestedKeys(const json& j) {
    if (j.is_object()) {
        size_t total = 0;
        for (auto& [key, value] : j.items()) {
            total += countJsonNestedKeys(value); // recurse
        }
        return total;
    }
    else if (j.is_number()) {
        return 1;
    }
    return 0;
}

json freadJson(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Could not open " + path);
    std::stringstream buffer;
    buffer << f.rdbuf(); // read into buffer
    f.close();
    json j = json::parse(buffer.str()); // read as json
    return j;
}

std::vector<std::string> listDirectories(const std::string& path) {
    std::vector<std::string> dirs;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA((path + "*").c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) return dirs;

    do {
        std::string name = fd.cFileName;
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..") {
            dirs.push_back(name);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return dirs;
}

std::vector<std::string> listFiles(const std::string& path) {
    std::vector<std::string> files;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA((path + "*").c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) return files;

    do {
        std::string name = fd.cFileName;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(name);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return files;
}

// Oh, say can you see?
std::array<std::string, 51> statesAbbreviations = {
    "AL", "AK", "AZ", "AR", "CA", "CO", //█████████████████████████████████████████████████████████
       "CT","DE","DC","FL","GA","HI",   //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
    "ID", "IL", "IN", "IA", "KS", "KY", //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
       "LA", "ME", "MD", "MA", "MI",    //█████████████████████████████████████████████████████████
    "MN", "MS", "MO", "MT", "NE", "NV", //
       "NH", "NJ", "NM", "NY", "NC",    //█████████████████████████████████████████████████████████
    "ND", "OH", "OK", "OR", "PE", "RI", //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
       "SC", "SD", "TN", "TX", "UT",    //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    "VT", "VI", "WA", "WV", "WI", "WY"  //█████████████████████████████████████████████████████████
    //
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
    //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
};