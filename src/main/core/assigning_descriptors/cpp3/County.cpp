#include "County.h"

#include <algorithm>
#include <sstream>

#include "Utils.h"
#include "Descriptor.h"

void County::recalculate() {
    descriptorsDemographics.fill(0.0);
    // Loop through the descriptors of which this county is a member
    for (size_t idx : descriptorIndices) {
        const auto& descriptor = (*descriptorsPtr)[idx];
        // Get the demographics of each
        const auto& descriptorDemographics = descriptor->getDemographics();
        for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
            descriptorsDemographics[i] += descriptorDemographics[i];
        }
    }
    // Clamp the demographics to [0, 1]
    for (size_t i = 0; i < descriptorsDemographics.size(); ++i) {
        descriptorsDemographics[i] = std::clamp(descriptorsDemographics[i], 0.0, 1.0);
    }

    // Determine score and missing demographics
    score = compareDemographics(demographics, descriptorsDemographics, "js");
    missingDemographics = subtract(demographics, descriptorsDemographics); // actual - predicted
    // Expected = 0.50, Actual = 0.25, Difference = 0.25. 25% missing
    // Expected = 0.25, Actual = 0.50, Difference = -0.25. -25% missing, or 25% excess
}

County::County(
    const std::string& name,
    const std::string& FIPS,
    size_t index,
    const uint32_t population,
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
    std::vector<std::unique_ptr<Descriptor>>* descriptorsPtr
) : name{name}, FIPS{FIPS}, index{index}, population{population},
    demographics{demographics}, descriptorsPtr{descriptorsPtr}
{
    this->recalculate();
}

County::County(
    const County& other
) : name{other.name}, FIPS{other.FIPS}, index{other.index}, population{other.population},
    neighborsIndices{other.neighborsIndices}, demographics{other.demographics},
    descriptorsPtr{other.descriptorsPtr}
{
    this->recalculate();
}

const std::string& County::getName() const noexcept {
    return name;
}

const std::string& County::getFIPS() const noexcept {
    return FIPS;
}

std::string County::getStateFIPS() const {
    return FIPS.substr(0, 2);
}

size_t County::getIndex() const noexcept {
    return index;
}

uint32_t County::getPopulation() const noexcept{
    return population;
}

double County::getScore() const noexcept {
    return score;
}

const std::unordered_set<size_t>& County::getNeighborsIndices() const noexcept {
    return neighborsIndices;
}

bool County::hasNeighbor(size_t neighborIndex) const noexcept {
    return neighborsIndices.count(neighborIndex) != 0;
}

void County::addNeighbor(size_t neighborIndex) noexcept {
    neighborsIndices.insert(neighborIndex);
}

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getDemographics() const noexcept {
    return demographics;
}

void County::setDescriptorsPtr(std::vector<std::unique_ptr<Descriptor>>* descriptorsPtr) {
    this->descriptorsPtr = descriptorsPtr;
    this->recalculate();
}

const std::unordered_set<size_t>& County::getDescriptorIndices() const noexcept {
    return descriptorIndices;
}

bool County::hasDescriptor(size_t descriptorIndex) const noexcept {
    return descriptorIndices.count(descriptorIndex) != 0;
}

bool County::hasDescriptor(const Descriptor& d) const {
    return this->hasDescriptor(d.getIndex());
}

void County::addDescriptor(size_t descriptorIndex) noexcept {
    descriptorIndices.insert(descriptorIndex);
    this->recalculate();
}

void County::addDescriptor(const Descriptor& d) {
    this->addDescriptor(d.getIndex());
}

void County::removeDescriptor(size_t descriptorIndex) noexcept {
    descriptorIndices.erase(descriptorIndex);
    this->recalculate();
}

void County::removeDescriptor(const Descriptor& d) {
    this->removeDescriptor(d.getIndex());
}

void County::addOrRemoveDescriptor(size_t descriptorIndex) noexcept {
    if (this->hasDescriptor(descriptorIndex))
        this->removeDescriptor(descriptorIndex);
    else
        this->addDescriptor(descriptorIndex);
}

void County::addOrRemoveDescriptor(const Descriptor& d) {
    this->addOrRemoveDescriptor(d.getIndex());
}

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getDescriptorsDemographics() const noexcept {
    return descriptorsDemographics;
}

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getMissingDemographics() const noexcept {
    return missingDemographics;
}

std::string County::to_string() const {
    std::ostringstream oss{};
    oss << this->name << " [" << this->FIPS << "] " << score << ";";
    return oss.str();
}

nlohmann::json County::to_json() const {
    std::vector<std::string> descriptorsNames{};
    for (const size_t index : descriptorIndices) {
        descriptorsNames.push_back((*descriptorsPtr)[index]->getName());
    }
    return nlohmann::json{
        { "name", name },
        { "FIPS", FIPS },
        { "state", stateFIPSToName[getStateFIPS()] },
        { "population", population },
        { "score", score },
        { "number_descriptors", descriptorIndices.size() },
        { "descriptors", descriptorsNames }
    };
}

bool County::operator==(const County& other) const {
    return this->name == other.name
        && this->FIPS == other.FIPS
        && this->population == other.population;
}

std::ostream& operator<<(std::ostream& os, const County& obj) {
    return os << obj.to_string();
}