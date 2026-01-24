#pragma once

#include "../FileHandling/DataFile.h"
#include <string>

class Item : public DataFile
{
public:
    Item() = default;
    Item(const std::string &datafilepath);

    // Getters for required fields
    std::string name() const;
    std::string description() const;
};
