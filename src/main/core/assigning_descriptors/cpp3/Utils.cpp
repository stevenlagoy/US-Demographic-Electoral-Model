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
    "ND", "OH", "OK", "OR", "PA", "RI", //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
       "SC", "SD", "TN", "TX", "UT",    //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    "VT", "VA", "WA", "WV", "WI", "WY"  //█████████████████████████████████████████████████████████
    //
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
    //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //
    //█████████████████████████████████████████████████████████████████████████████████████████████
    //▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
};

std::map<std::string, std::string> stateNameToAbbr = {
    {"alabama", "AL"},
    {"alaska", "AK"},
    {"arizona", "AZ"},
    {"arkansas", "AR"},
    {"california", "CA"},
    {"colorado", "CO"},
    {"connecticut", "CT"},
    {"delaware", "DE"},
    {"district_of_columbia", "DC"},
    {"florida", "FL"},
    {"georgia", "GA"},
    {"hawaii", "HI"},
    {"idaho", "ID"},
    {"illinois", "IL"},
    {"indiana", "IN"},
    {"iowa", "IA"},
    {"kansas", "KS"},
    {"kentucky", "KY"},
    {"louisiana", "LA"},
    {"maine", "ME"},
    {"maryland", "MD"},
    {"massachusetts", "MA"},
    {"michigan", "MI"},
    {"minnesota", "MN"},
    {"mississippi", "MS"},
    {"missouri", "MO"},
    {"montana", "MT"},
    {"nebraska", "NE"},
    {"nevada", "NV"},
    {"new_hampshire", "NH"},
    {"new_jersey", "NJ"},
    {"new_mexico", "NM"},
    {"new_york", "NY"},
    {"north_carolina", "NC"},
    {"north_dakota", "ND"},
    {"ohio", "OH"},
    {"oklahoma", "OK"},
    {"oregon", "OR"},
    {"pennsylvania", "PA"},
    {"rhode_island", "RI"},
    {"south_carolina", "SC"},
    {"south_dakota", "SD"},
    {"tennessee", "TN"},
    {"texas", "TX"},
    {"utah", "UT"},
    {"vermont", "VT"},
    {"virginia", "VA"},
    {"washington", "WA"},
    {"west_virginia", "WV"},
    {"wisconsin", "WI"},
    {"wyoming", "WY"}
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

string progressBar(double percent, int width, bool showPercent) {
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