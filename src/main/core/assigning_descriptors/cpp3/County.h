#ifndef COUNTY_H
#define COUNTY_H

#include <array>
#include <string>
#include <unordered_set>
#include <stdint.h>
#include <ostream>
#include <memory>
#include "../../../../lib/json.hpp"

#include "Constants.h"

class Descriptor; // Forward-declare Descriptor

class County {
private:
    const std::string name;
    const std::string FIPS;
    const size_t index = std::numeric_limits<size_t>::max(); // Initialize to max value to detect errors
    const uint32_t population;
    std::unordered_set<size_t> neighborsIndices;
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    std::vector<std::unique_ptr<Descriptor>>* descriptorsPtr;
    double score;

    std::unordered_set<size_t> descriptorIndices;
    std::array<double, NUMBER_DEMOGRAPHICS> descriptorsDemographics;

    void recalculate();
public:
    County(
        const std::string& name,
        const std::string& FIPS,
        size_t index,
        const uint32_t population,
        const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
        std::vector<std::unique_ptr<Descriptor>>* descriptorsPtr
    );
    County(const County& other);
    ~County() = default;

    const std::string& getName() const noexcept;
    const std::string& getFIPS() const noexcept;
    std::string getStateFIPS() const;
    size_t getIndex() const noexcept;
    uint32_t getPopulation() const noexcept;
    double getScore() const noexcept;
    const std::unordered_set<size_t>& getNeighborsIndices() const noexcept;
    bool hasNeighbor(size_t neighborIndex) const noexcept;
    void addNeighbor(size_t neighborIndex) noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    void setDescriptorsPtr(std::vector<std::unique_ptr<Descriptor>>* descriptorsPtr);

    const std::unordered_set<size_t>& getDescriptorIndices() const noexcept;
    bool hasDescriptor(size_t descriptorIndex) const noexcept;
    bool hasDescriptor(const Descriptor& d) const;
    void addDescriptor(size_t descriptorIndex) noexcept;
    void addDescriptor(const Descriptor& d);
    void removeDescriptor(size_t descriptorIndex) noexcept;
    void removeDescriptor(const Descriptor& d);
    void addOrRemoveDescriptor(size_t descriptorIndex) noexcept;
    void addOrRemoveDescriptor(const Descriptor& d);

    std::string to_string() const;
    nlohmann::json to_json() const;

    bool operator==(const County& other) const;
    friend std::ostream& operator<<(std::ostream& os, const County& obj);
};

#endif