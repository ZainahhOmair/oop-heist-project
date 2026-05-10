#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
    std::string name; //cannot write std in header file
    int value;
public:
    Item(std::string n, int v) {
        name = n;
        value = v;
    }
    int getValue() {
        return value;
    }
    std::string getName() {
        return name;
    }
};

#endif