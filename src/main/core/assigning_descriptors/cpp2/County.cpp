#include "County.h"

#include <sstream>
#include <iomanip>

// Determine descDemographics from descriptor membership
void County::recalculate() {
    descDemographics.fill(0.0);
    for (size_t idx: descriptorIndices) {
        const auto& desc = descriptorsRef->at(idx);
        const auto& effects = desc.getEffects();
        for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
            descDemographics[i] += effects[i];
        }
    }
}

County::County(
    const std::string& name, std::string countyFIPS, uint32_t population,
    const std::array<double, NUMBER_DEMOGRAPHICS>& demographics,
    const std::array<Descriptor, NUMBER_DESCRIPTORS>* descriptorsRef
) : descriptorsRef{descriptorsRef}, name{name}, countyFIPS{countyFIPS}, population{population}, demographics{demographics}
{
    addDescriptor(0);
    recalculate();
}

const std::string& County::getName() const noexcept { return name; }

// First two digits in countyFIPS
std::string County::getStateFIPS() const noexcept {
    return countyFIPS.substr(0, 2);
}

const std::string& County::getCountyFIPS() const noexcept { return countyFIPS; }

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

void County::addDescriptor(size_t descIndex) noexcept {
    descriptorIndices.emplace(descIndex);
    recalculate();
}

void County::removeDescriptor(size_t descIndex) noexcept {
    if (descriptorIndices.erase(descIndex)) recalculate();
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

std::ostream& operator<<(std::ostream& os, const County& obj) {
    os << obj.toString();
    return os;
}