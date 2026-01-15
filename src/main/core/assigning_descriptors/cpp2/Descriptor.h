#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <string>
#include <array>
#include <iostream>

#include "Constants.h"
#include "Utils.h"

// Forward declare County
class County;

class Descriptor {
private:
    const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef; // Reference from main of all demographic names
    std::string name;
    std::array<double, NUMBER_DEMOGRAPHICS> effects{}; // Initialize with zeroes
    std::vector<const County*> memberCounties;
    bool membershipModifiable; // Descriptors with modifiable membership can have the counties they apply to change.
    void recalculateSpecificity();
    void recalculateLocality();
    double specificityScore;
    double localityScore;
public:
    Descriptor();
    explicit Descriptor(
        const std::string& name,
        const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef,
        bool membershipModifiable = true
    );
    ~Descriptor() = default;
    std::string getName() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS >& getEffects() const noexcept;
    void setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects);
    double getEffect(size_t index) const;
    void setEffect(size_t index, const double value);
    void addEffect(size_t index, const double value);
    bool hasAnyEffect() const;
    std::vector<const County*> getMemberCounties() const noexcept;
    bool hasMemberCounty(const County& county) const noexcept;
    bool addMemberCounty(const County* county);
    bool removeMemberCounty(const County* county);
    bool addOrRemoveMemberCounty(const County* county);
    double getSpecificityScore() const noexcept;
    double getLocalityScore() const noexcept;
    bool isMembershipModifiable() const noexcept;
    bool operator==(const Descriptor& other) const;
    std::string toString() const noexcept;
    json toJson() const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif