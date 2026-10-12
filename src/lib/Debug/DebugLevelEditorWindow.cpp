#include "DebugLevelEditorWindow.h"
#include "DebugEntityEditorWindow.h"
#include "LevelV1.h"
#include <algorithm>
#include <filesystem>
#include <imgui.h>

namespace
{
    // Appends "<root>/<folder>" for each subfolder of root (a "/"-prefixed asset path), sorted.
    void collectEntityFolders(const std::string &root, std::vector<std::string> &out)
    {
        std::error_code error;
        std::vector<std::string> found;
        for (const auto &entry : std::filesystem::directory_iterator(std::filesystem::path(root).relative_path(), error))
        {
            if (entry.is_directory())
                found.push_back(root + "/" + entry.path().filename().string());
        }
        std::sort(found.begin(), found.end());
        out.insert(out.end(), found.begin(), found.end());
    }
}

// m_entities is only bound by reference here; the base never reads it until setLevel() runs.
DebugLevelEditorWindow::DebugLevelEditorWindow()
    : DataFileBySchemaDebugWindow("Level Editor", m_entities, DataFileSchema::LevelEntities)
{
}

void DebugLevelEditorWindow::setLevel(const LevelV1 &level, const std::string &levelDirectory)
{
    m_entityPaths.clear();
    collectEntityFolders("/assets/DataFiles/Entities", m_entityPaths);
    collectEntityFolders("/" + levelDirectory + "/DataFiles/Entities", m_entityPaths);
    m_entities = level.getEntities();
    // The level stores its path without a leading "/", which DataFile::save needs to find the write location.
    m_entities.setpath("/" + levelDirectory + "/entities.json");
    loadFromDataFile();
}

bool DebugLevelEditorWindow::renderCustomField(const std::string &id, nlohmann::json &value, const std::string &widget)
{
    // Only the "path" of each entry in the entities array is a dropdown.
    const std::string suffix = "].path";
    const bool isEntityPath = id.rfind("root.entities[", 0) == 0 &&
                              id.size() > suffix.size() &&
                              id.compare(id.size() - suffix.size(), suffix.size(), suffix) == 0;
    if (!isEntityPath || !value.is_string())
        return false;

    const std::string current = value.get<std::string>();
    if (ImGui::BeginCombo(widget.c_str(), current.empty() ? "<select>" : current.c_str()))
    {
        for (const auto &option : m_entityPaths)
        {
            if (ImGui::Selectable(option.c_str(), option == current))
                value = option;
        }
        ImGui::EndCombo();
    }
    return true;
}

void DebugLevelEditorWindow::renderFooter()
{
    if (ImGui::Button("Create new entity") && m_entityEditorWindow)
    {
        m_entityEditorWindow->show();
    }
}
