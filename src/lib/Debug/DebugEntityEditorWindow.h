#pragma once

#include "DataFileBySchemaDebugWindow.h"
#include <string>
#include <vector>

class DebugEntityEditorWindow : public DataFileBySchemaDebugWindow
{
public:
    explicit DebugEntityEditorWindow(DataFile &dataFile);

    void show();

    // Writes the entity as <path>/<entity name>/character.json. `path` uses the same
    // form as entity paths, e.g. "/assets/DataFiles/Entities". Returns true on success.
    bool save_entity_to(const std::string &path);

    // Level folder used by "Save to level", e.g. "assets/Levels/spaceship".
    void setLevelDirectory(const std::string &levelDirectory) { m_levelDirectory = levelDirectory; }

protected:
    void renderFooter() override;
    bool renderCustomField(const std::string &id, nlohmann::json &value, const std::string &widget) override;

private:
    void refreshLayerFiles();

    std::vector<std::string> m_layerFiles;
    std::string m_levelDirectory;
};
