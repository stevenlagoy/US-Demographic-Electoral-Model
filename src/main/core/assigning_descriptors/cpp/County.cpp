#include "County.h"
#include <algorithm>

const vector<float>& County::recalculate() {
    descDemographics.assign(demographics.size(), 0.0f);
    for (auto* desc : descriptors) {
        const auto& effects = desc->getEffects();
        for (size_t i = 0; i < effects.size(); ++i) {
            descDemographics[i] += effects[i];
        }
    }
    return descDemographics;
}

County::County(string name, string state, int population, vector<float> demographics, vector<Descriptor*> descriptors)
    : name{name}, state{state}, population{population}, demographics{demographics}, descDemographics{}, descriptors{descriptors} {
    this->recalculate();
}

string County::getName() const {
    return this->name;
}

string County::getState() const {
    return this->state;
}

const vector<float>& County::getDemographics() const {
    return demographics;    
}

const vector<float>& County::descriptorDemographics() const {
    return descDemographics;
}

float County::getDemographic(size_t demographic) const {
    return this->demographics[demographic];
}

const vector<Descriptor*>& County::getDescriptors() const {
    return this->descriptors;
}

bool County::hasDescriptor(Descriptor* d) const {
    return std::count(descriptors.begin(), descriptors.end(), d);
}

const vector<Descriptor*>& County::addDescriptor(Descriptor* d) {
    if (!hasDescriptor(d)) {
        this->descriptors.emplace_back(d);
        this->recalculate();
    }
    return this->descriptors;
}

const vector<Descriptor*>& County::removeDescriptor(Descriptor* d) {
    auto newEnd = std::remove(descriptors.begin(), descriptors.end(), d);
    descriptors.erase(newEnd, descriptors.end());
    this->recalculate();
    return this->descriptors;
}

string County::toString() const {
    string result = "";
    result += this->name + ", " + this->state + " | population: " + std::to_string(population) + " | descriptors: [";
    for (size_t i = 0; i < descriptors.size(); i++) {
        result += descriptors.at(i)->getName();
        if (i + 1 < descriptors.size()) result += ", ";
    }
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