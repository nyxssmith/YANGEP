#pragma once

#include "../FileHandling/DataFile.h"
#include <string>
#include "Action.h"
class Item : public DataFile
{
public:
    Item() = default;
    Item(const std::string &datafilepath);

    // Getters for required fields
    std::string name() const;
    std::string description() const;
    std::string provides_action() const; // Action folder name, empty if none
    std::string icon() const;

    // Validates required fields and loads the provided action, if any; false if fields are missing
    bool init();

    bool hasProvidedAction() const;
    Action *getProvidedAction();
    // Runs the provided action; returns false if the item provides none
    bool doProvidedAction();

    Action provided_action;

private:
    bool has_provided_action = false;
};
