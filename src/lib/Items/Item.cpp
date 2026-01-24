#include "Item.h"
#include <stdexcept>

// Constructor with datafilepath - loads the item data from file
Item::Item(const std::string &datafilepath) : DataFile(datafilepath)
{
    // DataFile constructor already calls load(datafilepath)

    // Validate that required fields exist
    if (!this->contains("name") || !(*this)["name"].is_string())
    {
        throw std::runtime_error("Item JSON must contain a 'name' field (string)");
    }

    if (!this->contains("description") || !(*this)["description"].is_string())
    {
        throw std::runtime_error("Item JSON must contain a 'description' field (string)");
    }
}

// Get the item's name
std::string Item::name() const
{
    return (*this)["name"].get<std::string>();
}

// Get the item's description
std::string Item::description() const
{
    return (*this)["description"].get<std::string>();
}
