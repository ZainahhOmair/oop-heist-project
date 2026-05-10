#include "InventoryManager.h"
#include <iostream>
using namespace std;

void InventoryManager::addItem(Item* item) { //to add item to inventory and print message
    items.push_back(item);
    cout << "Item added: " << item->getName() << endl;
}

int InventoryManager::getTotalValue() { //to calculate total value of items in inventory
    int total = 0;
    for (auto i : items)
        total += i->getValue();
    return total;
}

// returns how many diamonds collected so far
int InventoryManager::getItemCount() {
    return items.size();
}

InventoryManager::~InventoryManager() {
    for (auto i : items)
        delete i;
}