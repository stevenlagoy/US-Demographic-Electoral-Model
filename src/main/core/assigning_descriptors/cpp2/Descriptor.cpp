#include "Descriptor.h"
#include <sstream>
#include <iostream>
#include <tuple>
#include <algorithm>

void Descriptor::recalculate() {
    score = std::accumulate(
        effects.cbegin(), effects.cend(), 0.0,
        [this](double total, double e) {
            return total + std::pow(e, 2);
        }
    );
}

Descriptor::Descriptor() : Descriptor("", nullptr) {}

Descriptor::Descriptor(
    const std::string& name,
    const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef,
    bool membershipModifiable
) : demographicsRef{demographicsRef}, name{name}, membershipModifiable{membershipModifiable} {
    recalculate();
}

std::string Descriptor::getName() const noexcept { return name; }

const std::array<double, NUMBER_DEMOGRAPHICS >& Descriptor::getEffects() const noexcept {
    return effects;
}

void Descriptor::setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects) {
    this->effects = effects;
    recalculate();
}

double Descriptor::getEffect(const size_t index) const {
    return effects[index];
}

void Descriptor::setEffect(const size_t index, const double value) {
    effects[index] = std::max(value, 0.0);
    recalculate();
}

void Descriptor::addEffect(const size_t index, const double value) {
    effects[index] = std::max(effects[index] + value, 0.0);
    recalculate();
}

bool Descriptor::hasAnyEffect() const {
    return 0.0 == std::accumulate(effects.cbegin(), effects.cend(), 0.0);
}

double Descriptor::getScore() const noexcept {
    return score;
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