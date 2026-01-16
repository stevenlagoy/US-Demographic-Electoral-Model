#ifndef COUNTY_H
#define COUNTY_H

#include <array>
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
    std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef; // Reference from main for all descriptors
    const std::string name;
    const std::string countyFIPS; // FIPS code of the county - first 2 digits are state fips
    const uint32_t population;
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    std::array<double, NUMBER_DEMOGRAPHICS> descDemographics;
    std::unordered_set<size_t> descriptorIndices{};
    const std::unordered_set<std::string> neighborCountyFIPS;
    double score;
public:
    void recalculate();
    County(
        const std::string& name, std::string countyFIPS, uint32_t population,
        const std::unordered_set<std::string> neighborCountyFIPS,
        const std::array<double, NUMBER_DEMOGRAPHICS>& demographics,
        std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef
    );
    County(const County& other);
    ~County() = default;
    void setDescriptorsRef(std::array<Descriptor, NUMBER_DESCRIPTORS>* ref);
    const std::string& getName() const noexcept;
    std::string getStateFIPS() const noexcept;
    uint32_t getPopulation() const noexcept;
    const std::string& getCountyFIPS() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDescriptorDemographics() const noexcept;
    const std::unordered_set<size_t>& getDescriptorIndices() const noexcept;
    bool hasDescriptor(size_t descIndex) const noexcept;
    bool hasDescriptor(const Descriptor desc) const noexcept;
    void addDescriptor(size_t descIndex) noexcept;
    void removeDescriptor(size_t descIndex) noexcept;
    void addOrRemoveDescriptor(size_t descIndex) noexcept; // Adds if not present, removes if present
    double getScore() const;
    std::unordered_set<std::string> getNeighborCountyFIPS() const noexcept;
    bool hasNeighbor(std::string neighborFIPS) const;
    bool hasNeighbor(const County& c) const;
    std::string toString() const;
    json toJson() const;
    bool operator==(const County& other) const;
    friend std::ostream& operator<<(std::ostream& os, const County& obj);
};

#endif