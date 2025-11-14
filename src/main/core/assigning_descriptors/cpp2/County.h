#ifndef COUNTY_H
#define COUNTY_H

#include <string>
#include <cstdint>
#include <array>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Constants.h"
#include "Descriptor.h"
#include "Utils.h"

class County {
private:
    const std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef; // Reference from main for all descriptors
    const std::string name;
    const std::string countyFIPS; // FIPS code of the county - first 2 digits are state fips
    const uint32_t population;
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    std::array<double, NUMBER_DEMOGRAPHICS> descDemographics;
    std::unordered_set<size_t> descriptorIndices{};
    void recalculate();
    double score;
public:
    County(
        const std::string& name, std::string countyFIPS, uint32_t population,
        const std::array<double, NUMBER_DEMOGRAPHICS>& demographics,
        const std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef
    );
    County(const County& other);
    const std::string& getName() const noexcept;
    std::string getStateFIPS() const noexcept;
    uint32_t getPopulation() const noexcept;
    const std::string& getCountyFIPS() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDescriptorDemographics() const noexcept;
    const std::unordered_set<size_t>& getDescriptorIndices() const noexcept;
    bool hasDescriptor(size_t descIndex) const noexcept;
    void addDescriptor(size_t descIndex) noexcept;
    void removeDescriptor(size_t descIndex) noexcept;
    void addOrRemoveDescriptor(size_t descIndex) noexcept; // Adds if not present, removes if present
    double getScore() const;
    std::string toString() const;
    json toJson() const;
    friend std::ostream& operator<<(std::ostream& os, const County& obj);
};

#endif