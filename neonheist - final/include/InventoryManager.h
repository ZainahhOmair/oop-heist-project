#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <vector>
#include "Item.h"

class InventoryManager {
    std::vector<Item*> items;
public:
    void addItem(Item* item);
    int getTotalValue();
    int getItemCount();
    ~InventoryManager();
};

#endif