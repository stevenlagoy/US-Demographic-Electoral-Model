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

template <size_t N>
void normalize(std::array<double, N>& arr, int level = 1) {
    double sum = std::accumulate(arr.cbegin(), arr.cend(), 0.0, [level](double acc, double val) { return acc + std::pow(val, level); });
    sum = std::pow(sum, (1.0/level));
    if (sum == 0.0) return;
    for (auto &v : arr) v /= sum;
}

double compareDemographics(const std::array<double, NUMBER_DEMOGRAPHICS>& expected, const std::array<double, NUMBER_DEMOGRAPHICS>& actual, std::string method) {

    // Check that vectors have same length
    if (expected.size() != actual.size()) {
        std::cerr << "Compared vectors have different sizes: " << std::to_string(expected.size()) << ", " << std::to_string(actual.size()) << "." << std::endl; 
        return 0.0;
    }

    // Copy vectors
    std::array<double, NUMBER_DEMOGRAPHICS> e = expected;
    std::array<double, NUMBER_DEMOGRAPHICS> a = actual;

    // Check if population disparity
    if (std::accumulate(a.begin(), a.end(), 0.0) == 0.0 &&
        std::accumulate(e.begin(), e.end(), 0.0) != 0.0) {
        return 0.0;
    }

    if (method == "l1") { // L1 Norm - Sum of absolute differences (Manhattan Distance)
        std::array<double, NUMBER_DEMOGRAPHICS> distances{};
        // L1-Normalize vectors
        normalize(e);
        normalize(a);
        for (size_t i = 0; i < distances.size(); i++) {
            distances[i] = abs(e[i] - a[i]);
        }
        double dist = std::accumulate(distances.begin(), distances.end(), 0.0);
        return std::clamp(1 - dist / 2, 0.0, 1.0); // Normalize to [0, 1]
    }
    else if (method == "l2") { // L2 Norm - Eucledian distance
        std::array<double, NUMBER_DEMOGRAPHICS> distances{};
        // L2-Normalize vectors
        normalize(e, 2);
        normalize(a, 2);
        for (size_t i = 0; i < distances.size(); i++) {
            distances[i] = pow(e[i] - a[i], 2);
        }
        double dist = sqrt(std::accumulate(distances.begin(), distances.end(), 0.0));
        return std::clamp(1 - dist / std::sqrt(2), 0.0, 1.0); // Normalize to [0, 1]
    }
    else if (method == "cosine") { // Cosine Similarities - Dot product
        // L2-Normalize vectors
        normalize(e, 2);
        normalize(a, 2);
        double dot = std::inner_product(e.begin(), e.end(), a.begin(), 0.0);
        return dot;
    }
    else if (method == "js") { // Jensen-Shannon Divergence
        auto kl = []<size_t N>(std::array<double, N> p, std::array<double, N> q) { // Kullback-Leibler
            std::array<double, N> d{};
            for(size_t i = 0; i < d.size(); ++i) {
                if (p[i] && q[i])
                    d[i] = p[i] * log2(p[i]/q[i]);
            }
            double sum = std::accumulate(d.begin(), d.end(), 0.0);
            return sum;
        };
        std::array<double, NUMBER_DEMOGRAPHICS> m{};
        // L1-Normalize vectors
        normalize(e);
        normalize(a);
        for (size_t i = 0; i < m.size(); ++i) {
            m[i] = (e[i] + a[i])/2;
        }
        double js = (kl(e, m) + kl(a, m)) / 2; // Get J-S divergence
        double sim = 1 - js; // Invert
        return std::clamp(sim, 0.0, 1.0);
    }
    else { // Invalid method
        throw std::invalid_argument("Unknown method: " + method);
    }
}

int randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max - 1);
    return dist(rng());
}

double randomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng());
}

bool randomChance(float chance) {
    if (chance < 0.0) return false;
    if (chance >= 1.0) return true;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng()) < chance;
}