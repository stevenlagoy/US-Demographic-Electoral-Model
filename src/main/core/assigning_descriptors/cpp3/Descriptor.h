#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "Constants.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <array>
#include <memory>
#include <ostream>
#include <limits>
#include "../../../../lib/json.hpp"

class County; // Forward-declare County

class Descriptor {
private:
    const std::string name;
    const size_t index = std::numeric_limits<size_t>::max(); // Initialize to max value to detect errors
    std::vector<std::unique_ptr<County>>* countiesPtr;
    std::unordered_set<size_t> memberCountiesIndices;
    std::array<double, NUMBER_DEMOGRAPHICS> demographics;
    
    const bool membershipModifiable = true;

    double score;

    void recalculate();
public:
    Descriptor() = delete;
    explicit Descriptor(
        const std::string name,
        const size_t index,
        std::vector<std::unique_ptr<County>>* countiesPtr,
        std::vector<size_t> memberCountiesIndices = {},
        bool membershipModifiable = true
    );
    ~Descriptor() = default;

    const std::string& getName() const noexcept;
    size_t getIndex() const noexcept;
    bool isMembershipModifiable() const noexcept;
    const std::unordered_set<size_t>& getMemberCountiesIndices() const noexcept;
    std::vector<std::string> getMemberCountiesFIPS() const;
    void clearMemberCounties() noexcept;
    bool hasMemberCounty(size_t countyIndex) const noexcept;
    void addMemberCounty(size_t countyIndex);
    void removeMemberCounty(size_t countyIndex);
    void addOrRemoveMemberCounty(size_t countyIndex);
    const std::array<double, NUMBER_DEMOGRAPHICS>& getDemographics() const noexcept;
    void setCountiesPtr(std::vector<std::unique_ptr<County>>* countiesPtr);
    
    double getScore() const noexcept;

    std::string to_string() const;
    nlohmann::json to_json() const;
    
    bool operator==(const Descriptor& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif