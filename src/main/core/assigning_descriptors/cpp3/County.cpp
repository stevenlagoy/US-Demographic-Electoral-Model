#include "County.h"

#include <algorithm>
#include <sstream>

void County::recalculate() {
    this->score = 0.0;
}

County::County(
    const std::string& name,
    const std::string& FIPS,
    const uint32_t population,
    std::unordered_set<std::string> neighbors,
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
    std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr
) : name{name}, FIPS{FIPS}, population{population},
    neighbors{neighbors}, demographics{demographics},
    descriptorsPtr{descriptorsPtr}
{
    this->recalculate();
}

County::County(
    const std::string& name,
    const std::string& FIPS,
    const uint32_t population,
    std::vector<std::string> neighbors,
    const std::array<double, NUMBER_DEMOGRAPHICS> demographics,
    std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr
) : name{name}, FIPS{FIPS}, population{population},
    neighbors{neighbors.begin(), neighbors.end()}, demographics{demographics},
    descriptorsPtr{descriptorsPtr}
{
    this->recalculate();
}

County::County(
    const County& other
) : name{other.name}, FIPS{other.FIPS}, population{other.population},
    neighbors{other.neighbors}, demographics{other.demographics},
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

uint32_t County::getPopulation() const noexcept{
    return population;
}

double County::getScore() const noexcept {
    return score;
}

const std::unordered_set<std::string>& County::getNeighbors() const noexcept {
    return neighbors;
}

bool County::hasNeighbor(const std::string& neighborFIPS) const noexcept {
    return neighbors.count(neighborFIPS) != 0;
}

bool County::hasNeighbor(const County& c) const noexcept {
    return hasNeighbor(c.FIPS);
}

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getDemographics() const noexcept {
    return demographics;
}

void County::setDescriptorsPtr(std::array<Descriptor, NUMBER_DESCRIPTORS>* const descriptorsPtr) {
    this->descriptorsPtr = descriptorsPtr;
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
}

void County::addDescriptor(const Descriptor& d) {
    this->addDescriptor(d.getIndex());
}

void County::removeDescriptor(size_t descriptorIndex) noexcept {
    descriptorIndices.erase(descriptorIndex);
}

void County::removeDescriptor(const Descriptor& d) {
    descriptorIndices.erase(d.getIndex());
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

std::string County::to_string() const {
    std::ostringstream oss{};
    oss << this->name << " [" << this->FIPS << "] " << score << ";";
    return oss.str();
}

nlohmann::json County::to_json() const {
    std::vector<std::string> descriptorsNames{};
    for (const size_t index : descriptorIndices) {
        descriptorsNames.push_back((*descriptorsPtr)[index].getName());
    }
    return nlohmann::json{
        { "name", name },
        { "FIPS", FIPS },
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