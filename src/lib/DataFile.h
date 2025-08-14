#pragma once

#include <nlohmann/json.hpp>
#include <string>

class DataFile : public nlohmann::json
{
private:
    std::string filename;

public:
    DataFile() = default;
    DataFile(const std::string &filename);

    // Load JSON from file
    bool load(const std::string &filename);
    bool load(); // Load from stored filename

    // Save JSON to file
    bool save(const std::string &filename) const;
    bool save() const; // Save to stored filename

    // Get/Set filename
    const std::string &getFilename() const;
    void setFilename(const std::string &filename);
};
