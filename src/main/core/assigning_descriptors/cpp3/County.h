#ifndef COUNTY_H
#define COUNTY_H

#include <array>
#include <string>
#include <unordered_set>
#include <stdint.h>
#include <ostream>
#include "../../../../lib/json.hpp"

#include "Constants.h"
#include "Descriptor.h"

class County {
private:
    const std::string name;
    const std::string FIPS;
    const uint32_t population;
    std::unordered_set<std::string> neighbors;
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsPtr;
    double score;

    std::unordered_set<size_t> descriptorIndices;
    std::array<double, NUMBER_DEMOGRAPHICS> descriptorDemographics;

    void recalculate();
public:
    County(
        const std::string& name,
        const std::string& FIPS,
        const uint32_t population,
        std::unordered_set<std::string> neighbors,
        const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
        std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr
    );
    County(
        const std::string& name,
        const std::string& FIPS,
        const uint32_t population,
        std::vector<std::string> neighbors,
        const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
        std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr
    );
    County(const County& other);
    ~County() = default;

    const std::string& getName() const noexcept;
    const std::string& getFIPS() const noexcept;
    uint32_t getPopulation() const noexcept;
    double getScore() const noexcept;
    const std::unordered_set<std::string>& getNeighbors() const noexcept;
    bool hasNeighbor(const std::string& neighborFIPS) const noexcept;
    bool hasNeighbor(const County& c) const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    void setDescriptorsPtr(std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr);

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