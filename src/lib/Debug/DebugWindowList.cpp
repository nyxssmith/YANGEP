#include "DebugWindowList.h"
#include "DataFileDebugWindow.h"
#include "DataFileBySchemaDebugWindow.h"
#include "DebugStateMachineEditorWindow.h"
#include <stdio.h>

DebugWindowEntry::DebugWindowEntry(int id, const std::string &path)
    : id(id), filepath(path)
{
}
DebugWindowEntry::~DebugWindowEntry() = default;
DebugWindowEntry::DebugWindowEntry(DebugWindowEntry &&) noexcept = default;
DebugWindowEntry &DebugWindowEntry::operator=(DebugWindowEntry &&) noexcept = default;

DebugWindowList::DebugWindowList()
    : m_nextID(1)
{
}
DebugWindowList::~DebugWindowList() = default;
DebugWindowList::DebugWindowList(DebugWindowList &&) noexcept = default;
DebugWindowList &DebugWindowList::operator=(DebugWindowList &&) noexcept = default;

int DebugWindowList::add(const std::string &filepath)
{
    return addImpl(filepath, nullptr);
}

int DebugWindowList::add(const std::string &filepath, DataFileSchema schema)
{
    return addImpl(filepath, &schema);
}

int DebugWindowList::addImpl(const std::string &filepath, const DataFileSchema *schema)
{
    // Check if this filepath already exists
    if (findIndexByPath(filepath) != -1)
    {
        printf("Debug window for '%s' already exists\n", filepath.c_str());
        return -1;
    }

    try
    {
        // Create new entry
        DebugWindowEntry entry(m_nextID, filepath);

        // Create DataFile
        entry.dataFile = std::make_unique<DataFile>(filepath);

        // Extract filename from path for window title
        std::string filename = filepath;
        size_t lastSlash = filepath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            filename = filepath.substr(lastSlash + 1);
        }

        // Create debug window with title
        std::string title = "Debug: " + filename;
        if (schema)
        {
            if (*schema == DataFileSchema::StateMachine)
                entry.window = std::make_unique<DebugStateMachineEditorWindow>(title, *entry.dataFile);
            else
                entry.window = std::make_unique<DataFileBySchemaDebugWindow>(title, *entry.dataFile, *schema);
        }
        else
            entry.window = std::make_unique<DataFileDebugWindow>(title, *entry.dataFile);

        int assignedID = m_nextID;
        m_windows.push_back(std::move(entry));
        m_nextID++;

        printf("Added debug window (ID: %d) for '%s'\n", assignedID, filepath.c_str());
        return assignedID;
    }
    catch (const std::exception &e)
    {
        printf("Failed to add debug window for '%s': %s\n", filepath.c_str(), e.what());
        return -1;
    }
}

bool DebugWindowList::removeByID(int id)
{
    int index = findIndexByID(id);
    if (index == -1)
    {
        printf("Debug window with ID %d not found\n", id);
        return false;
    }

    printf("Removed debug window (ID: %d) for '%s'\n", id, m_windows[index].filepath.c_str());
    m_windows.erase(m_windows.begin() + index);
    return true;
}

bool DebugWindowList::removeByPath(const std::string &filepath)
{
    int index = findIndexByPath(filepath);
    if (index == -1)
    {
        printf("Debug window for '%s' not found\n", filepath.c_str());
        return false;
    }

    int id = m_windows[index].id;
    printf("Removed debug window (ID: %d) for '%s'\n", id, filepath.c_str());
    m_windows.erase(m_windows.begin() + index);
    return true;
}

void DebugWindowList::renderAll()
{
    for (auto &entry : m_windows)
    {
        if (entry.window)
        {
            entry.window->render();
        }
    }
}

size_t DebugWindowList::count() const
{
    return m_windows.size();
}

void DebugWindowList::clear()
{
    printf("Clearing all debug windows (%zu total)\n", m_windows.size());
    m_windows.clear();
}

int DebugWindowList::findIndexByID(int id) const
{
    for (size_t i = 0; i < m_windows.size(); ++i)
    {
        if (m_windows[i].id == id)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int DebugWindowList::findIndexByPath(const std::string &filepath) const
{
    for (size_t i = 0; i < m_windows.size(); ++i)
    {
        if (m_windows[i].filepath == filepath)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}
