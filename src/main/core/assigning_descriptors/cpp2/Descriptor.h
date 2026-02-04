#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <string>
#include <array>
#include <iostream>
#include <unordered_set>

#include "Constants.h"
#include "Utils.h"

// Forward declare County
class County;

class Descriptor {
private:
    std::vector<std::unique_ptr<County>>* countiesRef; // Reference from main for all counties
    const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef; // Reference from main of all demographic names
    std::string name;
    std::array<double, NUMBER_DEMOGRAPHICS> effects{}; // Initialize with zeroes
    std::unordered_set<County*> memberCounties;
    bool membershipModifiable; // Descriptors with modifiable membership can have the counties they apply to change.
    void recalculateSpecificity();
    void recalculateLocality();
    double specificityScore;
    double localityScore;
public:
    Descriptor() = default;
    explicit Descriptor(
        std::vector<std::unique_ptr<County>>* countiesRef,
        const std::string& name,
        const std::array<std::string, NUMBER_DEMOGRAPHICS>* demographicsRef,
        bool membershipModifiable = true
    );
    ~Descriptor() = default;
    void setCountiesRef(std::vector<std::unique_ptr<County>>* ref);
    std::string getName() const noexcept;
    const std::array<double, NUMBER_DEMOGRAPHICS >& getEffects() const noexcept;
    void setEffects(const std::array<double, NUMBER_DEMOGRAPHICS>& effects);
    double getEffect(size_t index) const;
    void setEffect(size_t index, const double value);
    void addEffect(size_t index, const double value);
    bool hasAnyEffect() const;
    const std::unordered_set<County*>& getMemberCounties() const noexcept;
    bool hasMemberCounty(County* county) const noexcept;
    void addMemberCounty(County* county);
    void removeMemberCounty(County* county);
    void addOrRemoveMemberCounty(County* county);
    void clearMemberCounties();
    double getSpecificityScore() const noexcept;
    double getLocalityScore() const noexcept;
    bool isMembershipModifiable() const noexcept;
    bool operator==(const Descriptor& other) const;
    std::string toString() const noexcept;
    json toJson() const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif