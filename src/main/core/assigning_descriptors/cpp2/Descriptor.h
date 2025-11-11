#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <string>
#include <array>
#include <iostream>

#include "Constants.h"

class Descriptor {
private:
    std::string name;
    std::array<double, NUMBER_DEMOGRAPHICS> effects{}; // Initialize with zeroes
    bool membershipModifiable; // Descriptors with modifiable membership can have the counties they apply to change.
public:
    Descriptor();
    Descriptor(const std::string& name, bool membershipModifiable = true);
    std::string getName() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS >& getEffects() const noexcept;
    void setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects);
    double getEffect(size_t index) const;
    void setEffect(size_t index, const double value);
    constexpr bool isMembershipModifiable() const noexcept;
    bool operator==(const Descriptor& other) const;
    std::string toString() const noexcept;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif