#include "Descriptor.h"
#include <sstream>
#include <iostream>
#include <tuple>
#include <algorithm>
#include <cmath>

#include "County.h" // Resolve forward declaration of County

void Descriptor::recalculateSpecificity() {
    if (effects.empty()) {
        specificityScore = 0.0;
        return;
    }
    specificityScore = std::accumulate(
        effects.cbegin(), effects.cend(), 0.0,
        [this](double total, double e) {
            return total + std::abs(std::pow(e, 2));
        }
    ) / (1 * 1 * static_cast<double>(effects.size()));
}

void Descriptor::recalculateLocality() {
    if (!membershipModifiable || memberCounties.empty()) {
        localityScore = membershipModifiable ? 0.0 : 1.0;
        return;
    }

    int numNeighbors = 0;
    // Make a temp iterable copy of memberCounties
    std::vector<const County*> members;
    for (auto& it : memberCounties) members.push_back(it);
    for (size_t i = 0; i < members.size(); ++i) {
        for (size_t j = i + 1; j < members.size(); ++j) {
            if (members[i]->hasNeighbor(*members[j])) numNeighbors++;
        }
    }
    double expectedNeighbors(memberCounties.size() * EXPECTED_NEIGHBORS_PER_COUNTY * 0.5);
    if (expectedNeighbors == 0) {
        // Should not occur after we check memberCounties.size != 0
        localityScore = 0.0;
        return;
    }
    if (numNeighbors > 0) {
        localityScore = numNeighbors / expectedNeighbors;
    }
    else {
        localityScore = 0.0; // Did this for debugging
    }
}

Descriptor::Descriptor(
    std::vector<std::unique_ptr<County>>* countiesRef,
    const std::string& name,
    const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef,
    bool membershipModifiable
)
    : countiesRef{countiesRef},
      demographicsRef{demographicsRef},
      name{name},
      memberCounties{},
      membershipModifiable{membershipModifiable},
      specificityScore{0.0},
      localityScore{membershipModifiable ? 0.0 : 1.0}
{
    recalculateSpecificity();
    recalculateLocality();
}

void Descriptor::setCountiesRef(std::vector<std::unique_ptr<County>>* ref) {
    countiesRef = ref;
}

std::string Descriptor::getName() const noexcept { return name; }

const std::array<double, NUMBER_DEMOGRAPHICS >& Descriptor::getEffects() const noexcept {
    return effects;
}

void Descriptor::setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects) {
    this->effects = effects;
    recalculateSpecificity();
}

double Descriptor::getEffect(const size_t index) const {
    return effects[index];
}

void Descriptor::setEffect(const size_t index, const double value) {
    effects[index] = std::max(value, 0.0);
    recalculateSpecificity();
}

void Descriptor::addEffect(const size_t index, const double value) {
    effects[index] = std::max(effects[index] + value, 0.0);
    recalculateSpecificity();
}

bool Descriptor::hasAnyEffect() const {
    double totalEffect = std::accumulate(effects.cbegin(), effects.cend(), 0.0);
    return totalEffect != 0.0;
}

const std::unordered_set<County*>& Descriptor::getMemberCounties() const noexcept {
    return memberCounties;
}

bool Descriptor::hasMemberCounty(County* county) const noexcept {
    return memberCounties.find(county) != memberCounties.end();
}

void Descriptor::addMemberCounty(County* county) {
    memberCounties.insert(county);
    recalculateLocality();
}

void Descriptor::removeMemberCounty(County* county) {
    memberCounties.erase(county);
    recalculateLocality();
}

void Descriptor::addOrRemoveMemberCounty(County* county) {
    hasMemberCounty(county) ? removeMemberCounty(county) : addMemberCounty(county);
}

void Descriptor::clearMemberCounties() {
    memberCounties.clear();
}

double Descriptor::getSpecificityScore() const noexcept {
    return specificityScore;
}

double Descriptor::getLocalityScore() const noexcept {
    return localityScore;
}

bool Descriptor::isMembershipModifiable() const noexcept { return membershipModifiable; }

bool Descriptor::operator==(const Descriptor& other) const {
    return std::tie(name, effects, membershipModifiable) == std::tie(other.name, other.effects, other.membershipModifiable);
}

// "1" : {0.01894, -0.24895, 0.85439, ...};
std::string Descriptor::toString() const noexcept {
    std::ostringstream oss;
    oss << "\"" << name << "\" : {";
    for (size_t i = 0; i < effects.size(); ++i) {
        oss << effects[i];
        if (i + 1 < effects.size()) oss << ", ";
    }
    oss << "};";
    return oss.str();
}

json Descriptor::toJson() const {
    json effectsJson = json::object();
    if (!demographicsRef) {
        return { { name, effectsJson } };
    }
    for (size_t i = 0; i < effects.size(); ++i) {
        const auto& key = (*demographicsRef)[i];
        effectsJson[key] = effects[i];
    }
    return { { name, effectsJson } };
}

std::ostream& operator<<(std::ostream& os, const Descriptor& obj) {
    os << obj.toString();
    return os;
}