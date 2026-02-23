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

// Eigen please don't segfault
#define EIGEN_DONT_ALIGN_STATICALLY
#define EIGEN_DONT_VECTORIZE
#include "../../../../lib/Eigen/Dense"
#include "../../../../lib/Eigen/Eigenvalues"

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

extern std::map<std::string, std::string> stateNameToCensusDivision;

extern std::map<std::string, std::string> censusDivisionToCensusRegion;

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

template<size_t N>
std::array<double, N> subtract(const std::array<double, N>& arr1, const std::array<double, N>& arr2) {
    std::array<double, N> res;
    for (size_t i = 0; i < arr1.size(); ++i) {
        res[i] = arr1[i] - arr2[i];
    }
    return res;
}

template<size_t N>
std::array<std::array<double, N>, N> vectorToSquareMatrix(const std::array<double, N>& vec) {
    // Multply vec by vec_transform for an N x N matrix
    std::array<std::array<double, N>, N> res;
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = 0; j <= i; ++j) { // Symmetric, so only need to do lower triangle
            res[i][j] = vec[i] * vec[j];
            res[j][i] = vec[j] * vec[i];
        }
    }
    return res;
}

std::vector<std::vector<double>> vectorToSquareMatrix(const std::vector<double>& vec);

template<size_t D>
inline Eigen::MatrixXd toEigen(const std::array<std::array<double, D>, D>& matrix) {
    Eigen::MatrixXd m(D, D);
    for (size_t i = 0; i < D; ++i)
        for (size_t j = 0; j < D; ++j)
            m(i, j) = matrix[i][j];
    return m;
}

struct PCAResult {
    Eigen::VectorXd eigenvalues;  // descending
    Eigen::MatrixXd eigenvectors; // columns
};

inline PCAResult computePCA(const Eigen::MatrixXd& cov, size_t D = NUMBER_DEMOGRAPHICS) {
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(cov);

    Eigen::VectorXd vals = solver.eigenvalues();
    Eigen::MatrixXd vecs = solver.eigenvectors();

    // Reverse order
    Eigen::VectorXd vals_desc(D);
    Eigen::MatrixXd vecs_desc(D, D);

    for (size_t i = 0; i < D; ++i) {
        vals_desc(i) = vals(D - 1 - i);
        vecs_desc.col(i) = vecs.col(D - 1 - i);
    }

    return { vals_desc, vecs_desc };
}

inline int chooseComponentCount(const Eigen::VectorXd& eigenvalues, double threshold = 0.999) {
    double total = eigenvalues.sum();
    double running{0.0};

    for (int i = 0; i < eigenvalues.size(); ++i) {
        running += eigenvalues(i);
        if (running / total >= threshold)
            return i + 1;
    }
    return eigenvalues.size();
}

inline Eigen::VectorXd projectCounty(const Eigen::VectorXd& residual, const Eigen::MatrixXd& eigenvectors, int k) {
    return eigenvectors.leftCols(k).transpose() * residual;
}

struct CountyPoint {
    Eigen::VectorXd z;
    double population;
    int cluster = -1;
};

template <size_t N>
double determinant(const std::array<std::array<double, N>, N>& matrix) {
    // Base case
    if constexpr (N == 1) return matrix[0][0];
    else if constexpr (N == 2) {
        double det = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
        return det;
    }
    else {
        // Recursive case
        double det{};
        bool step = false;
        for (size_t i = 0; i < matrix.size(); ++i) {
            std::array<std::array<double, N-1>, N-1> submat;
            for (size_t row = 1; row < matrix.size(); ++row) {
                for (size_t col = 0; col < matrix.size(); ++col) {
                    if (col == i) continue; // Skip own column
                    submat[row-1][col > i ? col - 1 : col] = matrix[row][col];
                }
            }
            double product = matrix[0][i] * determinant(submat);
            det = step ? det - product : det + product;
            step = !step;
        }
        return det;
    }
}

template <size_t N>
std::array<std::array<double, N>, N> identity() {
    std::array<std::array<double, N>, N> identityMatrix{};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (i == j) {
                identityMatrix[i][j] = 1.0;
            }
        }
    }
    return identityMatrix;
}

#endif
