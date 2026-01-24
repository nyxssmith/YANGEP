#include "Inventory.h"

// Constructor - initializes inventory with N slots
Inventory::Inventory(size_t size) : capacity(size)
{
    items.resize(size, std::nullopt);
}

// Add an item to the first available slot
bool Inventory::addItem(const Item &item)
{
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (!items[i].has_value())
        {
            items[i] = item;
            return true;
        }
    }
    return false; // Inventory is full
}

// Remove item by index
bool Inventory::removeItem(size_t index)
{
    if (index >= items.size())
    {
        return false;
    }

    if (items[index].has_value())
    {
        items[index] = std::nullopt;
        return true;
    }

    return false; // Slot was already empty
}

// Remove item by Item reference (removes first matching item)
bool Inventory::removeItem(const Item &item)
{
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i].has_value())
        {
            // Compare items by their JSON content
            if (items[i].value().dump() == item.dump())
            {
                items[i] = std::nullopt;
                return true;
            }
        }
    }
    return false; // Item not found
}

// Get item by index
std::optional<Item> Inventory::getItem(size_t index) const
{
    if (index >= items.size())
    {
        return std::nullopt;
    }

    return items[index];
}

// Get inventory capacity
size_t Inventory::getCapacity() const
{
    return capacity;
}

// Get number of items currently in inventory
size_t Inventory::getItemCount() const
{
    size_t count = 0;
    for (const auto &item : items)
    {
        if (item.has_value())
        {
            ++count;
        }
    }
    return count;
}

// Change the size of the inventory
void Inventory::changeSize(size_t newSize)
{
    capacity = newSize;

    // If growing, just resize and add empty slots
    if (newSize > items.size())
    {
        items.resize(newSize, std::nullopt);
    }
    // If shrinking, resize (will truncate items at the end)
    else if (newSize < items.size())
    {
        items.resize(newSize);
    }
}
