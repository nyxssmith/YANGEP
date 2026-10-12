#pragma once
// Forward declaration used instead of including the full header to reduce compilation dependencies
// #include "DataFileDebugWindow.h"
#include "DataFile.h"
#include <string>
#include <vector>
#include <memory>

class DebugWindow;
enum class DataFileSchema;
struct DebugWindowEntry
{
    int id;
    std::string filepath;
    std::unique_ptr<DataFile> dataFile;
    std::unique_ptr<DebugWindow> window;

    DebugWindowEntry(int id, const std::string &path);
    // defaulted in the .cpp where DataFileDebugWindow is a complete type
    ~DebugWindowEntry();
    DebugWindowEntry(DebugWindowEntry &&) noexcept;
    DebugWindowEntry &operator=(DebugWindowEntry &&) noexcept;
};

class DebugWindowList
{
public:
    DebugWindowList();
    // defaulted in the .cpp where DataFileDebugWindow is a complete type
    ~DebugWindowList();

    // m_windows holds move-only entries; only allow moving the whole list.
    DebugWindowList(const DebugWindowList &) = delete;
    DebugWindowList &operator=(const DebugWindowList &) = delete;
    DebugWindowList(DebugWindowList &&) noexcept;
    DebugWindowList &operator=(DebugWindowList &&) noexcept;

    // Add a new debug window for a file path
    // Returns the ID of the added window, or -1 if failed
    int add(const std::string &filepath);

    // Same as add, but opens a schema-driven editor window for the file
    int add(const std::string &filepath, DataFileSchema schema);

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

    int addImpl(const std::string &filepath, const DataFileSchema *schema);

    // Find window index by ID
    int findIndexByID(int id) const;

    // Find window index by path
    int findIndexByPath(const std::string &filepath) const;
};
