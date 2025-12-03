#pragma once

#include <nlohmann/json.hpp>
#include <string>

/**
 * @class RealConfigFile
 * @brief A JSON configuration file reader that works without Cute Framework's VFS
 *
 * Unlike DataFile which uses CF's virtual filesystem, RealConfigFile reads directly
 * from the actual filesystem using standard C++ file I/O. This allows reading
 * configuration files before the Cute Framework window and VFS are initialized.
 *
 * Use this for early initialization configs like window settings.
 * Use DataFile for runtime asset loading after the app is initialized.
 */
class RealConfigFile : public nlohmann::json
{
private:
    std::string path;

public:
    RealConfigFile() = default;
    RealConfigFile(const std::string &path);

    // Load JSON from file (using standard filesystem, not CF VFS)
    bool load(const std::string &path);
    bool load(); // Load from stored path

    // Save JSON to file (using standard filesystem, not CF VFS)
    bool save(const std::string &path) const;
    bool save() const; // Save to stored path

    // Get/Set path
    const std::string &getpath() const;
    void setpath(const std::string &path);

    // Helper to resolve relative paths from executable location
    static std::string getExecutableDir();
    static std::string resolvePathFromExecutable(const std::string &relative_path);
};
