#include "County.h"
#include <iostream>
#include <algorithm>

// Determine descDemographics from descriptor membership
void County::recalculate() {
    descDemographics.fill(0.0);
    for (size_t idx: descriptorIndices) {
        const auto& desc = descriptorsRef->at(idx);
        const auto& effects = desc.getEffects();
        for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
            descDemographics[i] = std::clamp(descDemographics[i] + effects[i], 0.0, 1.0);
        }
    }
    double totalEffects = std::accumulate(descDemographics.cbegin(), descDemographics.cend(), 0.0);
    if (totalEffects == 0.0) {
        score = 0.0;
        return;
    }
    score = compareDemographics(demographics, descDemographics, "js");
}

County::County(
    const std::string& name, std::string countyFIPS, uint32_t population,
    const std::vector<std::string> neighborCountyFIPS,
    const std::array<double, NUMBER_DEMOGRAPHICS>& demographics,
    const std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef
) : descriptorsRef{descriptorsRef}, name{name}, countyFIPS{countyFIPS}, population{population}, demographics{demographics}, neighborCountyFIPS{neighborCountyFIPS}
{
    addDescriptor(0); // Add the national descriptor
    recalculate();
}

County::County(const County& other)
    : descriptorsRef(other.descriptorsRef),
      name(other.name),
      countyFIPS(other.countyFIPS),
      population(other.population),
      demographics(other.demographics),
      descDemographics(other.descDemographics),
      descriptorIndices(other.descriptorIndices),
      neighborCountyFIPS(other.neighborCountyFIPS),
      score(other.score)
{}

const std::string& County::getName() const noexcept { return name; }

// First two digits in countyFIPS
std::string County::getStateFIPS() const noexcept {
    return countyFIPS.substr(0, 2);
}

const std::string& County::getCountyFIPS() const noexcept { return countyFIPS; }

uint32_t County::getPopulation() const noexcept { return population; }

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getDemographics() const noexcept {
    return demographics;
}

const std::array<double, NUMBER_DEMOGRAPHICS>& County::getDescriptorDemographics() const noexcept {
    return descDemographics;
}

const std::unordered_set<size_t>& County::getDescriptorIndices() const noexcept {
    return descriptorIndices;
}

bool County::hasDescriptor(size_t descIndex) const noexcept {
    return descriptorIndices.find(descIndex) != descriptorIndices.end();
}

bool County::hasDescriptor(const Descriptor desc) const noexcept {
    auto it = std::find(descriptorsRef->cbegin(), descriptorsRef->cend(), desc);
    size_t index{0};
    if (it != descriptorsRef->cend()) {
        index = std::distance(descriptorsRef->cbegin(), it);
    }
    else return false;
    return hasDescriptor(index);
}

void County::addDescriptor(size_t descIndex) noexcept {    
    descriptorIndices.emplace(descIndex);
    recalculate();
}

void County::removeDescriptor(size_t descIndex) noexcept {
    if (descriptorIndices.erase(descIndex)) recalculate();
}

void County::addOrRemoveDescriptor(size_t descIndex) noexcept {
    if (hasDescriptor(descIndex)) {
        removeDescriptor(descIndex);
        // std::cout << "Removed descriptor " << descIndex << " from county " << countyFIPS << "\n";
    }
    else {
        addDescriptor(descIndex);
        // std::cout << "Added descriptor " << descIndex << " to county " << countyFIPS << "\n";
    }
}

double County::getScore() const {
    return score;
}

std::vector<std::string> County::getNeighborCountyFIPS() const noexcept {
    return neighborCountyFIPS;
}

bool County::hasNeighbor(std::string neighborFIPS) const {
    return std::find(neighborCountyFIPS.cbegin(), neighborCountyFIPS.cend(), neighborFIPS) != neighborCountyFIPS.cend();
}

bool County::hasNeighbor(const County& c) const {
    return this->hasNeighbor(c.getCountyFIPS());
}

// "Autauga County" (01001) : {"USA", "AL", "1", "2", ...};
std::string County::toString() const {
    std::ostringstream oss;
    oss << "\"" << name << "\" (" << countyFIPS << ") : {";
    bool first = true;
    for (size_t idx : descriptorIndices) {
        if (!first) oss << ", ";
        first = false;
        oss << (*descriptorsRef)[idx].getName();
    }
    oss << "};";
    return oss.str();
}

json County::toJson() const {
    json descriptorArr = json::array();
    for (size_t idx : descriptorIndices) {
        descriptorArr.push_back((*descriptorsRef)[idx].getName());
    }
    json res = {
        { countyFIPS, {
            { "descriptors", descriptorArr },
            { "accuracy", score }
        }}
    };
    return res;
}

std::ostream& operator<<(std::ostream& os, const County& obj) {
    os << obj.toString();
    return os;
}