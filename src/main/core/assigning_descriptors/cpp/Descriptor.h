#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <map>
using std::map;
#include <iostream>

class Descriptor {
private:
    string name;
    vector<float> effects;
    bool fixed;
    // Fixed descriptors cannot be added or removed from counties after the initital assignments
    // Fixed descriptors CAN have their effects modified
public:
    Descriptor(string name, vector<float> effects = {}, bool fixed = false);
    string getName() const;
    const vector<float>& getEffects() const;
    float getEffect(size_t index) const;
    float setEffect(size_t index, float effect);
    float addEffect(size_t index, float effect);
    bool isFixed() const;
    bool operator==(const Descriptor& other) const;
    string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif