#include "Descriptor.h"

Descriptor::Descriptor(string name, vector<float> effects, bool fixed)
    : name{name}, effects{effects}, fixed{fixed} {}

string Descriptor::getName() const {
    return this->name;
}

const vector<float>& Descriptor::getEffects() const {
    return this->effects;
}

float Descriptor::getEffect(size_t index) const {
    return this->effects[index];
}

float Descriptor::setEffect(size_t index, float effect) {
    this->effects[index] = effect;
    return effect;
}

float Descriptor::addEffect(size_t index, float effect) {
    float prior = this->effects[index];
    return this->setEffect(index, prior + effect);
}

bool Descriptor::isFixed() const {
    return this->fixed;
}

bool Descriptor::operator==(const Descriptor& other) const {
    return this->name == other.name &&
           this->effects == other.effects &&
           this->fixed == other.fixed;
}

string Descriptor::toString() const {
    string result = "";
    result += "\"" + name + "\" : {";
    for (const auto& val : effects) {
        if (val != 0.0)
            result += std::to_string(val) + ", ";
    }
    result += "};";
    return result;
}

std::ostream& operator<<(std::ostream& os, const Descriptor& obj) {
    os << obj.toString();
    return os;
}