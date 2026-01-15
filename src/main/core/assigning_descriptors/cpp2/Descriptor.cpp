#include "Descriptor.h"
#include <sstream>
#include <iostream>
#include <tuple>
#include <algorithm>
#include <cmath>

#include "County.h" // Resolve forward declaration of County

void Descriptor::recalculateSpecificity() {
    specificityScore = std::accumulate(
        effects.cbegin(), effects.cend(), 0.0,
        [this](double total, double e) {
            return total + std::abs(std::pow(e, 2));
        }
    ) / (1 * 1 * effects.size());
}

void Descriptor::recalculateLocality() {
    if (!membershipModifiable) {
        localityScore = 1.0;
        return;
    }
    if (memberCounties.size() == 0) {
        localityScore = 0.0;
        return;
    }

    int numNeighbors = 0;
    for (const auto& c1 : memberCounties) {
        for (const auto& c2 : memberCounties) {
            if (c1->hasNeighbor(*c2)) numNeighbors++;
        }
    }
    double expectedNeighbors(memberCounties.size() * EXPECTED_NEIGHBORS_PER_COUNTY);
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

Descriptor::Descriptor() : Descriptor("", nullptr) {}

Descriptor::Descriptor(
    const std::string& name,
    const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef,
    bool membershipModifiable
) : demographicsRef{demographicsRef}, name{name}, memberCounties{}, membershipModifiable{membershipModifiable} {
    recalculateSpecificity();
    recalculateLocality();
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

std::vector<const County*> Descriptor::getMemberCounties() const noexcept {
    return memberCounties;
}

bool Descriptor::hasMemberCounty(const County& county) const noexcept {
    const auto& it = std::find_if(memberCounties.cbegin(), memberCounties.cend(), [county](const auto* c) {
        return c != nullptr && *c == county;
    });
    return it != memberCounties.cend();
}

bool Descriptor::addMemberCounty(const County* county) {
    bool present = hasMemberCounty(*county);
    if (!present) {
        memberCounties.push_back(county);
        recalculateLocality();
    }
    return !present;
}

bool Descriptor::removeMemberCounty(const County* county) {
    bool present = hasMemberCounty(*county);
    if (present) {
        memberCounties.erase(std::remove(memberCounties.begin(), memberCounties.end(), county), memberCounties.end());
        recalculateLocality();
    }
    return present;
}

bool Descriptor::addOrRemoveMemberCounty(const County* county) {
    if (hasMemberCounty(*county)) {
        removeMemberCounty(county);
        return false;
    }
    else {
        addMemberCounty(county);
        return true;
    }
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