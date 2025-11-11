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
    vector<double> effects;
    bool fixed;
    // Fixed descriptors cannot be added or removed from counties after the initital assignments
    // Fixed descriptors CAN have their effects modified
public:
    Descriptor(string name, vector<double> effects = {}, bool fixed = false);
    string getName() const;
    const vector<double>& getEffects() const;
    double getEffect(size_t index) const;
    double setEffect(size_t index, double effect);
    double addEffect(size_t index, double effect);
    bool isFixed() const;
    bool operator==(const Descriptor& other) const;
    string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Descriptor& obj);
};

#endif