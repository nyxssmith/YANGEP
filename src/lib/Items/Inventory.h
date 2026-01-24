#pragma once

#include "Item.h"
#include <vector>
#include <optional>

class Inventory
{
private:
    std::vector<std::optional<Item>> items;
    size_t capacity;

public:
    Inventory(size_t size);

    // Add an item to the first available slot
    bool addItem(const Item &item);

    // Remove item by index
    bool removeItem(size_t index);

    // Remove item by Item reference (removes first matching item)
    bool removeItem(const Item &item);

    // Get item by index
    std::optional<Item> getItem(size_t index) const;

    // Get inventory capacity
    size_t getCapacity() const;

    // Get number of items currently in inventory
    size_t getItemCount() const;

    // Change the size of the inventory
    void changeSize(size_t newSize);
};
