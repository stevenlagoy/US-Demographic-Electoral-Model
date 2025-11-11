#ifndef COUNTY_H
#define COUNTY_H

#include <string>
using std::string;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include "Descriptor.h"
#include <ostream>
#include <unordered_set>
using std::unordered_set;

class County {
private:
    string name;
    string state;
    int population;
    vector<double> demographics;
    vector<double> descDemographics;
    unordered_set<Descriptor*> descriptors;
    bool shouldRecalculate = true;
public:
    County(string name, string state, int population, vector<double> demographics, unordered_set<Descriptor*> descriptors = {});
    string getName() const;
    string getState() const;
    const vector<double>& getDemographics() const;
    const vector<double>& descriptorDemographics();
    double getDemographic(size_t demographic) const;
    const unordered_set<Descriptor*>& getDescriptors() const;
    bool hasDescriptor(Descriptor* d) const;
    const unordered_set<Descriptor*>& addDescriptor(Descriptor* d);
    const unordered_set<Descriptor*>& removeDescriptor(Descriptor* d);
    const vector<double>& recalculate();
    string toString() const;
    bool operator<(const County& other) const;
    friend std::ostream& operator<<(std::ostream& os, const County& obj);
};

#endif