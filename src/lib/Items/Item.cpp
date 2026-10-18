#include "Item.h"
#include <cstdio>

// Constructor with datafilepath - loads the item data from file
Item::Item(const std::string &datafilepath)
{
    if (!load(datafilepath))
    {
        printf("Item: ERROR: Failed to load item from %s\n", datafilepath.c_str());
        return;
    }
    init();
}

bool Item::init()
{
    // DataFile inherits from nlohmann::json, so use it directly as the item config
    if (!this->contains("name") || !(*this)["name"].is_string() ||
        !this->contains("description") || !(*this)["description"].is_string())
    {
        printf("Item: ERROR: item json missing required fields (name or description)\n");
        return false;
    }

    // Load the provided action if specified in JSON
    has_provided_action = false;
    std::string actionName = provides_action();
    if (!actionName.empty())
    {
        std::string fullPath = "/assets/DataFiles/Actions/" + actionName;
        has_provided_action = provided_action.loadFromFolder(fullPath);
        if (has_provided_action)
        {
            printf("Item: Loaded provided action '%s' from '%s'\n", actionName.c_str(), fullPath.c_str());
        }
        else
        {
            printf("Item: WARNING: Failed to load provided action '%s' from '%s'\n", actionName.c_str(), fullPath.c_str());
        }
    }
    initialized = true;
    return true;
}

std::string Item::provides_action() const
{
    if (this->contains("provides_action") && (*this)["provides_action"].is_string())
    {
        return (*this)["provides_action"].get<std::string>();
    }
    return "";
}

std::string Item::icon() const
{
    if (!initialized)
    {
        return "";
    }
    if (this->contains("icon") && (*this)["icon"].is_string())
    {
        return (*this)["icon"].get<std::string>();
    }
    return "";
}

bool Item::hasProvidedAction() const
{
    return has_provided_action;
}

Action *Item::getProvidedAction()
{
    return has_provided_action ? &provided_action : nullptr;
}

bool Item::doProvidedAction()
{
    if (!has_provided_action)
    {
        return false;
    }
    provided_action.doAction();
    return true;
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
