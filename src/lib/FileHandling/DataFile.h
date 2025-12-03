#pragma once

#include <nlohmann/json.hpp>
#include <string>

class DataFile : public nlohmann::json
{
private:
    std::string path;

public:
    DataFile() = default;
    DataFile(const std::string &path);

    // Load JSON from file
    bool load(const std::string &path);
    bool load(); // Load from stored path

    // Save JSON to file
    bool save(const std::string &path) const;
    bool save() const; // Save to stored path

    // Get/Set path
    const std::string &getpath() const;
    void setpath(const std::string &path);
};
