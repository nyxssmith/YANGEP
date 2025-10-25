#include "RealConfigFile.h"
#include <fstream>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

// Constructor with path
RealConfigFile::RealConfigFile(const std::string &path) : path(path)
{
    load(path);
}

// Load JSON from file using standard C++ filesystem
bool RealConfigFile::load(const std::string &path)
{
    try
    {
        // Try to open the file
        std::ifstream file(path);

        if (!file.is_open())
        {
            // Try resolving path relative to executable
            std::string resolved_path = resolvePathFromExecutable(path);
            file.open(resolved_path);

            if (!file.is_open())
            {
                std::cerr << "RealConfigFile: Failed to open file: " << path << std::endl;
                std::cerr << "Also tried: " << resolved_path << std::endl;
                return false;
            }
        }

        // Read and parse JSON
        nlohmann::json::operator=(nlohmann::json::parse(file));

        // Store the path after successful loading
        this->path = path;

        file.close();
        return true;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "RealConfigFile: JSON parse error in " << path << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "RealConfigFile: Error loading " << path << ": " << e.what() << std::endl;
        return false;
    }
}

// Load JSON from stored path
bool RealConfigFile::load()
{
    if (path.empty())
    {
        std::cerr << "RealConfigFile: Cannot load, no path set" << std::endl;
        return false;
    }
    return load(path);
}

// Save JSON to file using standard C++ filesystem
bool RealConfigFile::save(const std::string &path) const
{
    try
    {
        // Ensure directory exists
        std::filesystem::path file_path(path);
        if (file_path.has_parent_path())
        {
            std::filesystem::create_directories(file_path.parent_path());
        }

        // Open file for writing
        std::ofstream file(path);
        if (!file.is_open())
        {
            std::cerr << "RealConfigFile: Failed to open file for writing: " << path << std::endl;
            return false;
        }

        // Write JSON with pretty formatting
        file << this->dump(2); // 2-space indentation

        file.close();

        // Update stored path after successful save
        const_cast<RealConfigFile *>(this)->path = path;

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "RealConfigFile: Error saving " << path << ": " << e.what() << std::endl;
        return false;
    }
}

// Save JSON to stored path
bool RealConfigFile::save() const
{
    if (path.empty())
    {
        std::cerr << "RealConfigFile: Cannot save, no path set" << std::endl;
        return false;
    }
    return save(path);
}

// Get path
const std::string &RealConfigFile::getpath() const
{
    return path;
}

// Set path
void RealConfigFile::setpath(const std::string &path)
{
    this->path = path;
}

// Get the directory where the executable is located
std::string RealConfigFile::getExecutableDir()
{
    std::string exe_path;

#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exe_path = buffer;
#elif defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        exe_path = buffer;
    }
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
    {
        exe_path = buffer;
    }
#endif

    // Get directory from full path
    std::filesystem::path path(exe_path);
    return path.parent_path().string();
}

// Resolve a relative path from the executable location
std::string RealConfigFile::resolvePathFromExecutable(const std::string &relative_path)
{
    std::filesystem::path exe_dir(getExecutableDir());
    std::filesystem::path resolved = exe_dir / relative_path;
    return resolved.string();
}
