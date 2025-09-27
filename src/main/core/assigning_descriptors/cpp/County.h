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

class County {
private:
    string name;
    string state;
    int population;
    vector<float> demographics;
    vector<float> descDemographics;
    vector<Descriptor*> descriptors;
    public:
    County(string name, string state, int population, vector<float> demogaphics, vector<Descriptor*> descriptors = {});
    string getName() const;
    string getState() const;
    const vector<float>& getDemographics() const;
    const vector<float>& descriptorDemographics() const;
    float getDemographic(size_t demographic) const;
    const vector<Descriptor*>& getDescriptors() const;
    bool hasDescriptor(Descriptor* d) const;
    const vector<Descriptor*>& addDescriptor(Descriptor* d);
    const vector<Descriptor*>& removeDescriptor(Descriptor* d);
    const vector<float>& recalculate();
    string toString() const;
    bool operator<(const County& other) const;
    friend std::ostream& operator<<(std::ostream& os, const County& obj);
};

#endif