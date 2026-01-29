#include "Descriptor.h"

#include <algorithm>
#include <sstream>

void Descriptor::recalculate() {
    score = 0.0;
}

Descriptor::Descriptor(
    const std::string name,
    const size_t index,
    std::vector<std::unique_ptr<County>>* countiesPtr,
    bool membershipModifiable
) : name{name}, index{index}, countiesPtr{countiesPtr}, membershipModifiable{membershipModifiable}
{
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

const std::unordered_set<std::string>& Descriptor::getMemberCountiesFIPS() const noexcept {
    return memberCountiesFIPS;
}

bool Descriptor::hasMemberCounty(const std::string& countyFIPS) const noexcept {
    return memberCountiesFIPS.count(countyFIPS) != 0;
}

void Descriptor::addMemberCounty(const std::string& countyFIPS) {
    memberCountiesFIPS.insert(countyFIPS);
}

void Descriptor::removeMemberCounty(const std::string& countyFIPS) {
    memberCountiesFIPS.erase(countyFIPS);
}

void Descriptor::addOrRemoveMemberCounty(const std::string& countyFIPS) {
    if (hasMemberCounty(countyFIPS))
        removeMemberCounty(countyFIPS);
    else
        addMemberCounty(countyFIPS);
}

const std::array<double, NUMBER_DEMOGRAPHICS>& Descriptor::getDemographics() const noexcept {
    return demographics;
}

double Descriptor::getScore() const noexcept {
    return score;
}

std::string Descriptor::to_string() const {
    std::ostringstream oss{};
    oss << this->name << " [" << this->index << "] {";
    bool first = true;
    for (const auto& cFIPS : memberCountiesFIPS) {
        oss << (first ? "" : ", ") << cFIPS;
    }
    oss << "}" << ";";
    return oss.str();
}

nlohmann::json Descriptor::to_json() const {
    return nlohmann::json{
        { "name", name },
        { "number_members", memberCountiesFIPS.size() },
        { "members", memberCountiesFIPS },
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