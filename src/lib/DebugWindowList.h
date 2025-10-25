#pragma once
#include "DataFileDebugWindow.h"
#include "DataFile.h"
#include <string>
#include <vector>
#include <memory>

struct DebugWindowEntry
{
    int id;
    std::string filepath;
    std::unique_ptr<DataFile> dataFile;
    std::unique_ptr<DataFileDebugWindow> window;

    DebugWindowEntry(int id, const std::string &path)
        : id(id), filepath(path)
    {
    }
};

class DebugWindowList
{
public:
    DebugWindowList();
    ~DebugWindowList() = default;

    // Add a new debug window for a file path
    // Returns the ID of the added window, or -1 if failed
    int add(const std::string &filepath);

    // Remove debug window by ID
    bool removeByID(int id);

    // Remove debug window by file path
    bool removeByPath(const std::string &filepath);

    // Render all debug windows
    void renderAll();

    // Get count of debug windows
    size_t count() const;

    // Clear all debug windows
    void clear();

private:
    std::vector<DebugWindowEntry> m_windows;
    int m_nextID;

    // Find window index by ID
    int findIndexByID(int id) const;

    // Find window index by path
    int findIndexByPath(const std::string &filepath) const;
};
