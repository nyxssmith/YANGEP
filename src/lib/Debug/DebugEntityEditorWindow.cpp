#include "DebugEntityEditorWindow.h"
#include <algorithm>
#include <cute.h>
#include <filesystem>
#include <imgui.h>

DebugEntityEditorWindow::DebugEntityEditorWindow(DataFile &dataFile)
    : DataFileBySchemaDebugWindow("Entity Editor", dataFile, DataFileSchema::EntitiesCharacter)
{
    // Hidden until opened from the level editor
    m_show = false;
    refreshLayerFiles();
}

void DebugEntityEditorWindow::show()
{
    refreshLayerFiles();
    m_show = true;
}

void DebugEntityEditorWindow::refreshLayerFiles()
{
    m_layerFiles.clear();
    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator("assets/Art/AnimationsSheets/idle", error))
    {
        if (entry.is_regular_file())
            m_layerFiles.push_back(entry.path().filename().string());
    }
    std::sort(m_layerFiles.begin(), m_layerFiles.end());
}

bool DebugEntityEditorWindow::renderCustomField(const std::string &id, nlohmann::json &value, const std::string &widget)
{
    // Only the "filename" of each entry in the layers array is a dropdown.
    const std::string suffix = "].filename";
    const bool isLayerFile = id.rfind("root.layers[", 0) == 0 &&
                             id.size() > suffix.size() &&
                             id.compare(id.size() - suffix.size(), suffix.size(), suffix) == 0;
    if (!isLayerFile || !value.is_string())
        return false;

    const std::string current = value.get<std::string>();
    if (ImGui::BeginCombo(widget.c_str(), current.empty() ? "<select>" : current.c_str()))
    {
        for (const auto &option : m_layerFiles)
        {
            if (ImGui::Selectable(option.c_str(), option == current))
                value = option;
        }
        ImGui::EndCombo();
    }
    return true;
}

bool DebugEntityEditorWindow::save_entity_to(const std::string &path)
{
    nlohmann::json content;
    if (!getValidWorking(content))
    {
        printf("DebugEntityEditorWindow: entity does not satisfy the schema, not saving\n");
        return false;
    }

    const std::string name = content.value("name", "");
    if (name.empty() || name.find_first_of("/\\") != std::string::npos || name == "." || name == "..")
    {
        printf("DebugEntityEditorWindow: invalid entity name '%s', not saving\n", name.c_str());
        return false;
    }

    std::string folder = path;
    while (folder.size() > 1 && folder.back() == '/')
        folder.pop_back();
    folder += "/" + name;

    // Mirror DataFile::save, which writes relative to the write directory.
    std::string writeFolder = folder;
    if (writeFolder.starts_with("/assets/"))
        writeFolder = writeFolder.substr(8);
    if (Cute::is_error(cf_fs_create_directory(writeFolder.c_str())))
    {
        printf("DebugEntityEditorWindow: failed to create folder '%s'\n", folder.c_str());
        return false;
    }

    DataFile file;
    static_cast<nlohmann::json &>(file) = content;
    return file.save(folder + "/character.json");
}

void DebugEntityEditorWindow::renderFooter()
{
    if (ImGui::Button("Save to global"))
    {
        save_entity_to("/assets/DataFiles/Entities");
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_levelDirectory.empty());
    if (ImGui::Button("Save to level"))
    {
        save_entity_to("/" + m_levelDirectory + "/DataFiles/Entities");
    }
    ImGui::EndDisabled();
}
