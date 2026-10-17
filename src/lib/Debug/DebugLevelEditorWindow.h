#pragma once

#include "DataFileBySchemaDebugWindow.h"
#include <string>
#include <vector>

class LevelV1;
class DebugEntityEditorWindow;
class DebugStateMachineEditorWindow;

class DebugLevelEditorWindow : public DataFileBySchemaDebugWindow
{
public:
    DebugLevelEditorWindow();

    // Copies the level's entities.json data into this window's form and gathers the
    // entity folders to pick from. levelDirectory is e.g. "assets/Levels/spaceship".
    void setLevel(const LevelV1 &level, const std::string &levelDirectory);
    // Window opened by the "Add entity" button.
    void setEntityEditorWindow(DebugEntityEditorWindow *window) { m_entityEditorWindow = window; }
    void setStateMachineEditorWindow(DebugStateMachineEditorWindow *window) { m_stateMachineEditorWindow = window; }

protected:
    void renderFooter() override;
    bool renderCustomField(const std::string &id, nlohmann::json &value, const std::string &widget) override;

private:
    DataFile m_entities;
    std::vector<std::string> m_entityPaths;
    DebugEntityEditorWindow *m_entityEditorWindow = nullptr;
    DebugStateMachineEditorWindow *m_stateMachineEditorWindow = nullptr;
};
