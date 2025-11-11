#include "Descriptor.h"
#include <sstream>
#include <tuple>

Descriptor::Descriptor() : Descriptor("") {}

Descriptor::Descriptor(const std::string& name, bool membershipModifiable)
    : name{name}, membershipModifiable{membershipModifiable} {}

std::string Descriptor::getName() const noexcept { return name; }

const std::array<double, NUMBER_DEMOGRAPHICS >& Descriptor::getEffects() const noexcept {
    return effects;
}

void Descriptor::setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects) {
    this->effects = effects;
}

double Descriptor::getEffect(const size_t index) const {
    return effects.at(index);
}

void Descriptor::setEffect(const size_t index, const double value) {
    effects.at(index) = value;
}

constexpr bool Descriptor::isMembershipModifiable() const noexcept { return membershipModifiable; }

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

std::ostream& operator<<(std::ostream& os, const Descriptor& obj) {
    os << obj.toString();
    return os;
}