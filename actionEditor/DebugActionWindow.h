#pragma once
#include "DebugWindow.h"
#include "DataFile.h"
#include <string>

class DebugActionWindow : public DebugWindow
{
public:
    DebugActionWindow(const std::string &title, const std::string &actionFolderPath);
    void render() override;

    // Getter for the action folder path
    const std::string &getActionFolderPath() const { return m_actionFolderPath; }

private:
    DataFile m_actionDataFile;
    std::string m_actionFolderPath;

    // Cached values in milliseconds
    float m_globalCooldown;
    float m_warmup;
    float m_cooldown;

    // Text input buffers
    char m_globalCooldownText[32];
    char m_warmupText[32];
    char m_cooldownText[32];
    char m_renameBuffer[64];

    void loadValues();
    void saveValues();
    void syncTextFields();
    bool renameAction(const std::string &newName);
};
