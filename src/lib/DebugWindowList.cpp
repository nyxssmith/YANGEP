#include "DebugWindowList.h"
#include "DebugPrint.h"
#include <stdio.h>

DebugWindowList::DebugWindowList()
    : m_nextID(1)
{
}

int DebugWindowList::add(const std::string &filepath)
{
    // Check if this filepath already exists
    if (findIndexByPath(filepath) != -1)
    {
        DebugPrint::Print("DebugWindows", "Debug window for '%s' already exists\n", filepath.c_str());
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
        entry.window = std::make_unique<DataFileDebugWindow>(title, *entry.dataFile);

        int assignedID = m_nextID;
        m_windows.push_back(std::move(entry));
        m_nextID++;

        DebugPrint::Print("DebugWindows", "Added debug window (ID: %d) for '%s'\n", assignedID, filepath.c_str());
        return assignedID;
    }
    catch (const std::exception &e)
    {
        DebugPrint::Print("DebugWindows", "Failed to add debug window for '%s': %s\n", filepath.c_str(), e.what());
        return -1;
    }
}

bool DebugWindowList::removeByID(int id)
{
    int index = findIndexByID(id);
    if (index == -1)
    {
        DebugPrint::Print("DebugWindows", "Debug window with ID %d not found\n", id);
        return false;
    }

    DebugPrint::Print("DebugWindows", "Removed debug window (ID: %d) for '%s'\n", id, m_windows[index].filepath.c_str());
    m_windows.erase(m_windows.begin() + index);
    return true;
}

bool DebugWindowList::removeByPath(const std::string &filepath)
{
    int index = findIndexByPath(filepath);
    if (index == -1)
    {
        DebugPrint::Print("DebugWindows", "Debug window for '%s' not found\n", filepath.c_str());
        return false;
    }

    int id = m_windows[index].id;
    DebugPrint::Print("DebugWindows", "Removed debug window (ID: %d) for '%s'\n", id, filepath.c_str());
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
    DebugPrint::Print("DebugWindows", "Clearing all debug windows (%zu total)\n", m_windows.size());
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
