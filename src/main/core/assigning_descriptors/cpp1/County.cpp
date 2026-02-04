#include "County.h"
#include <algorithm>

const vector<double>& County::recalculate() {
    descDemographics.assign(demographics.size(), 0.0f);
    for (auto* desc : descriptors) {
        const auto& effects = desc->getEffects();
        for (size_t i = 0; i < effects.size(); ++i) {
            descDemographics[i] += effects[i];
        }
    }
    shouldRecalculate = false;
    return descDemographics;
}

County::County(string name, string state, int population, vector<double> demographics, unordered_set<Descriptor*> descriptors)
    : name{name}, state{state}, population{population}, demographics{demographics}, descDemographics{}, descriptors{descriptors} {
    shouldRecalculate = true;
}

string County::getName() const {
    return name;
}

string County::getState() const {
    return state;
}

const vector<double>& County::getDemographics() const {
    return demographics;    
}

const vector<double>& County::descriptorDemographics() {
    if (shouldRecalculate) recalculate();
    return descDemographics;
}

double County::getDemographic(size_t demographic) const {
    return demographics[demographic];
}

const unordered_set<Descriptor*>& County::getDescriptors() const {
    return descriptors;
}

bool County::hasDescriptor(Descriptor* d) const {
    return descriptors.find(d) != descriptors.end();
}

const unordered_set<Descriptor*>& County::addDescriptor(Descriptor* d) {
    auto [it, inserted] = descriptors.insert(d);
    if (inserted) shouldRecalculate = true;
    return descriptors;
}

const unordered_set<Descriptor*>& County::removeDescriptor(Descriptor* d) {
    if (descriptors.erase(d) > 0) shouldRecalculate = true;
    return descriptors;
}

string County::toString() const {
    string result = "";
    result += name + ", " + state + " | population: " + std::to_string(population) + " | descriptors: [";
    for (auto* desc : descriptors) {
        result += desc->getName() + ", ";
    }
    if (!descriptors.empty()) result.erase(result.size()-2);
    result += "];";
    return result;
}

bool County::operator<(const County& other) const {
    return std::tie(state, name) < std::tie(other.state, other.name);
}

std::ostream& operator<<(std::ostream& os, const County& obj) {
    os << obj.toString();
    return os;
}