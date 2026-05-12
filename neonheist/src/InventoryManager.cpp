#include "InventoryManager.h"
#include <iostream>
using namespace std;

void InventoryManager::addItem(Item* item) {
    items.push_back(item);
    cout << "Item added: " << item->getName() << endl;
}

int InventoryManager::getTotalValue() {
    int total = 0;
    for (auto i : items)
        total += i->getValue();
    return total;
}

int InventoryManager::getItemCount() {
    return items.size();
}

InventoryManager::~InventoryManager() {
    for (auto i : items)
        delete i;
}