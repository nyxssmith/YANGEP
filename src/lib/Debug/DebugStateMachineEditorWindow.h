#pragma once

#include "DataFileBySchemaDebugWindow.h"
#include <string>
#include <vector>

class DebugStateMachineEditorWindow : public DataFileBySchemaDebugWindow
{
public:
    explicit DebugStateMachineEditorWindow(DataFile &dataFile);
    DebugStateMachineEditorWindow(const std::string &title, DataFile &dataFile);

    void show();

    // Loads a state machine JSON from the given path (e.g. "assets/DataFiles/StateMachines/wander.json").
    bool loadStateMachine(const std::string &path);

    // Resets the editor to a starter state machine template.
    void resetToDefault();

    // Saves the state machine as <path>/<name>.json. Returns true on success.
    bool save_state_machine_to(const std::string &path);

    // Level folder used by "Save to level", e.g. "assets/Levels/spaceship".
    void setLevelDirectory(const std::string &levelDirectory) { m_levelDirectory = levelDirectory; }

protected:
    void renderFooter() override;
    void renderArrayBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id) override;

private:
    void refreshAllowedStates();
    void refreshAvailableFiles();
    nlohmann::json createDefaultInputsForState(const std::string &stateName);
    void renderStateInputs(const std::string &stateName, nlohmann::json &stateJson, const std::string &id, nlohmann::json &allStates);

    std::vector<std::string> m_allowedStates;
    std::vector<std::string> m_existingFiles;
    std::string m_levelDirectory;
    int m_selectedNewStateType = 0;
    int m_selectedFileToLoad = 0;
};
