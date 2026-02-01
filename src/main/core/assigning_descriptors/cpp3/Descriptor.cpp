#include "Descriptor.h"

#include <algorithm>
#include <sstream>

#include "County.h"

void Descriptor::recalculate() {
    // Determine this descriptor's demographics based on the average member counties
    demographics.fill(0.0);
    uint64_t membersTotalPopulation{0};
    // Loop through member counties
    for (const auto& cIdx : memberCountiesIndices) {
        const auto& county = (*countiesPtr)[cIdx];
        membersTotalPopulation += county->getPopulation();
        // Get each member's demographics
        const auto& countyDemographics = county->getDemographics();
        for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
            // Add each demographic weighted by county population
            demographics[i] += countyDemographics[i] * county->getPopulation();
        }
    }
    for (size_t i = 0; i < NUMBER_DEMOGRAPHICS; ++i) {
        // Normalize per population
        demographics[i] /= membersTotalPopulation;
    }
}

Descriptor::Descriptor(
    const std::string name,
    const size_t index,
    std::vector<std::unique_ptr<County>>* countiesPtr,
    std::vector<size_t> memberCountiesIndices,
    bool membershipModifiable
) : name{name}, index{index}, countiesPtr{countiesPtr},
    membershipModifiable{membershipModifiable}
{
    for (auto& idx : memberCountiesIndices) {
        this->memberCountiesIndices.insert(idx);
    }
    this->recalculate();
}

const std::string& Descriptor::getName() const noexcept {
    return name;
}

size_t Descriptor::getIndex() const noexcept {
    return index;
}

bool Descriptor::isMembershipModifiable() const noexcept {
    return membershipModifiable;
}

const std::unordered_set<size_t>& Descriptor::getMemberCountiesIndices() const noexcept {
    return memberCountiesIndices;
}

std::vector<std::string> Descriptor::getMemberCountiesFIPS() const {
    std::vector<std::string> memberCountiesFIPS;
    for (const auto& cIdx : memberCountiesIndices) {
        const auto& county = (*countiesPtr)[cIdx];
        memberCountiesFIPS.push_back(county->getFIPS());
    }
    return memberCountiesFIPS;
}

void Descriptor::clearMemberCounties() noexcept {
    memberCountiesIndices.clear();
}

bool Descriptor::hasMemberCounty(size_t countyIndex) const noexcept {
    return memberCountiesIndices.count(countyIndex) != 0;
}

void Descriptor::addMemberCounty(size_t countyIndex) {
    memberCountiesIndices.insert(countyIndex);
    this->recalculate();
}

void Descriptor::removeMemberCounty(size_t countyIndex) {
    memberCountiesIndices.erase(countyIndex);
    this->recalculate();
}

void Descriptor::addOrRemoveMemberCounty(size_t countyIndex) {
    if (hasMemberCounty(countyIndex))
        removeMemberCounty(countyIndex);
    else
        addMemberCounty(countyIndex);
}

const std::array<double, NUMBER_DEMOGRAPHICS>& Descriptor::getDemographics() const noexcept {
    return demographics;
}

void Descriptor::setCountiesPtr(std::vector<std::unique_ptr<County>>* countiesPtr) {
    this->countiesPtr = countiesPtr;
    this->recalculate();
}

double Descriptor::getScore() const noexcept {
    return score;
}

std::string Descriptor::to_string() const {
    std::ostringstream oss{};
    oss << this->name << " [" << this->index << "] {";
    bool first = true;
    for (const auto& cFIPS : getMemberCountiesFIPS()) {
        oss << (first ? "" : ", ") << cFIPS;
    }
    oss << "}" << ";";
    return oss.str();
}

nlohmann::json Descriptor::to_json() const {
    return nlohmann::json{
        { "name", name },
        { "number_members", memberCountiesIndices.size() },
        { "members", getMemberCountiesFIPS() },
        { "score", score }
    };
}

bool Descriptor::operator==(const Descriptor& other) const {
    return this->name == other.name
        && this->index == other.index;
}

std::ostream& operator<<(std::ostream& os, const Descriptor& obj) {
    return os << obj.to_string();
}