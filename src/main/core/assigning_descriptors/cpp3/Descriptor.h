#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "Constants.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <array>
#include <memory>
#include <ostream>
#include "../../../../lib/json.hpp"

// Forward declare County
class County;

class Descriptor {
private:
    const std::string name;
    const size_t index;
    std::unordered_set<std::string> memberCountiesFIPS;
    std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    std::vector<std::unique_ptr<County>>* countiesPtr;
    
    const bool membershipModifiable = true;

    double score;

    void recalculate();
public:
    Descriptor() = default;
    explicit Descriptor(
        const std::string name,
        const size_t index,
        std::vector<std::unique_ptr<County>>* countiesPtr,
        bool membershipModifiable = true
    );
    ~Descriptor() = default;

    const std::string& getName() const noexcept;
    size_t getIndex() const noexcept;
    bool isMembershipModifiable() const noexcept;
    const std::unordered_set<std::string>& getMemberCountiesFIPS() const noexcept;
    bool hasMemberCounty(const std::string& countyFIPS) const noexcept;
    void addMemberCounty(const std::string& countyFIPS);
    void removeMemberCounty(const std::string& countyFIPS);
    void addOrRemoveMemberCounty(const std::string& countyFIPS);
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    
    double getScore() const noexcept;

    std::string to_string() const;
    nlohmann::json to_json() const;
    
    bool operator==(const Descriptor& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif