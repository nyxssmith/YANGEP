#include "DebugActionWindow.h"
#include <imgui.h>
#include <cute.h>
#include <stdio.h>
#include <fstream>
#include <filesystem>

DebugActionWindow::DebugActionWindow(const std::string &title, const std::string &actionFolderPath)
    : DebugWindow(title), m_actionFolderPath(actionFolderPath),
      m_globalCooldown(0.0f), m_warmup(0.0f), m_cooldown(0.0f)
{
    // Load the action.json file
    std::string actionFilePath = actionFolderPath + "/action.json";
    m_actionDataFile.load(actionFilePath);

    // Initialize text buffers
    memset(m_globalCooldownText, 0, sizeof(m_globalCooldownText));
    memset(m_warmupText, 0, sizeof(m_warmupText));
    memset(m_cooldownText, 0, sizeof(m_cooldownText));
    memset(m_renameBuffer, 0, sizeof(m_renameBuffer));

    // Initialize rename buffer with current action name
    if (m_actionDataFile.contains("name"))
    {
        std::string currentName = m_actionDataFile["name"].get<std::string>();
        snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", currentName.c_str());
    }

    loadValues();
    syncTextFields();

    m_show = true; // Open the window by default
}

void DebugActionWindow::loadValues()
{
    // Load values from JSON (convert from ms to ms, they're already in milliseconds)
    m_globalCooldown = m_actionDataFile.value("global_cooldown", 0.0f);
    m_warmup = m_actionDataFile.value("warmup", 0.0f);
    m_cooldown = m_actionDataFile.value("cooldown", 0.0f);
}

void DebugActionWindow::syncTextFields()
{
    snprintf(m_globalCooldownText, sizeof(m_globalCooldownText), "%.0f", m_globalCooldown);
    snprintf(m_warmupText, sizeof(m_warmupText), "%.0f", m_warmup);
    snprintf(m_cooldownText, sizeof(m_cooldownText), "%.0f", m_cooldown);
}

void DebugActionWindow::saveValues()
{
    // Update JSON with current values
    m_actionDataFile["global_cooldown"] = m_globalCooldown;
    m_actionDataFile["warmup"] = m_warmup;
    m_actionDataFile["cooldown"] = m_cooldown;

    // Get the real filesystem path
    std::string actionFilePath = m_actionFolderPath + "/action.json";
    std::string realPath = actionFilePath;
    if (realPath.starts_with("/assets/"))
    {
        realPath = realPath.substr(1); // Remove leading "/" to get "assets/..."
    }

    // Save directly to real filesystem
    try
    {
        std::ofstream file(realPath);
        if (file.is_open())
        {
            file << m_actionDataFile.dump(2);
            file.close();
            printf("DebugActionWindow: Saved action data to %s\n", realPath.c_str());
        }
        else
        {
            printf("DebugActionWindow: Failed to open %s for writing\n", realPath.c_str());
        }
    }
    catch (const std::exception &e)
    {
        printf("DebugActionWindow: Error saving action data: %s\n", e.what());
    }
}

void DebugActionWindow::render()
{
    if (!m_show)
        return;

    ImGui::Begin(m_title.c_str(), &m_show, 0);

    // Display and edit action name
    if (m_actionDataFile.contains("name"))
    {
        std::string actionName = m_actionDataFile["name"].get<std::string>();
        ImGui::Text("Action Name:");
        ImGui::InputText("##ActionName", m_renameBuffer, sizeof(m_renameBuffer), 0);
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
        {
            std::string newName = m_renameBuffer;
            if (!newName.empty() && newName != actionName)
            {
                renameAction(newName);
            }
        }
        ImGui::Separator();
    }

    // Global Cooldown slider (0-10000ms / 0-10s)
    ImGui::Text("Global Cooldown (ms)");
    if (ImGui::SliderFloat("##GlobalCooldownSlider", &m_globalCooldown, 0.0f, 10000.0f))
    {
        syncTextFields();
    }

    // Global Cooldown text input (allows values beyond slider range)
    if (ImGui::InputText("##GlobalCooldownInput", m_globalCooldownText, sizeof(m_globalCooldownText), 0))
    {
        m_globalCooldown = atof(m_globalCooldownText);
    }

    ImGui::Spacing();

    // Warmup slider (0-10000ms / 0-10s)
    ImGui::Text("Warmup (ms)");
    if (ImGui::SliderFloat("##WarmupSlider", &m_warmup, 0.0f, 10000.0f))
    {
        syncTextFields();
    }

    // Warmup text input
    if (ImGui::InputText("##WarmupInput", m_warmupText, sizeof(m_warmupText), 0))
    {
        m_warmup = atof(m_warmupText);
    }

    ImGui::Spacing();

    // Cooldown slider (0-10000ms / 0-10s)
    ImGui::Text("Cooldown (ms)");
    if (ImGui::SliderFloat("##CooldownSlider", &m_cooldown, 0.0f, 10000.0f))
    {
        syncTextFields();
    }

    // Cooldown text input
    if (ImGui::InputText("##CooldownInput", m_cooldownText, sizeof(m_cooldownText), 0))
    {
        m_cooldown = atof(m_cooldownText);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Save button
    if (ImGui::Button("Save"))
    {
        saveValues();
    }

    ImGui::SameLine();

    // Reload button
    if (ImGui::Button("Reload"))
    {
        loadValues();
        syncTextFields();
    }

    ImGui::End();
}

bool DebugActionWindow::renameAction(const std::string &newName)
{
    // Get the current action name and folder
    std::string oldName = m_actionDataFile["name"].get<std::string>();

    // Extract the parent directory path
    size_t lastSlash = m_actionFolderPath.find_last_of('/');
    if (lastSlash == std::string::npos)
    {
        printf("DebugActionWindow: Invalid folder path\n");
        return false;
    }

    std::string parentPath = m_actionFolderPath.substr(0, lastSlash);
    std::string newFolderPath = parentPath + "/" + newName;

    // Check if target folder already exists
    CF_Stat stat;
    if (!cf_is_error(cf_fs_stat(newFolderPath.c_str(), &stat)))
    {
        printf("DebugActionWindow: Action '%s' already exists\n", newName.c_str());
        return false;
    }

    // Update the action.json name field
    m_actionDataFile["name"] = newName;
    saveValues();

    // Use system to rename the folder (CF doesn't have a rename function)
    std::string oldDiskPath = m_actionFolderPath;
    std::string newDiskPath = newFolderPath;

    // Remove the /assets prefix for disk paths
    if (oldDiskPath.substr(0, 7) == "/assets")
    {
        oldDiskPath = "assets" + oldDiskPath.substr(7);
    }
    if (newDiskPath.substr(0, 7) == "/assets")
    {
        newDiskPath = "assets" + newDiskPath.substr(7);
    }

    // Rename the folder on disk
    if (std::rename(oldDiskPath.c_str(), newDiskPath.c_str()) != 0)
    {
        printf("DebugActionWindow: Failed to rename folder from %s to %s\n", oldDiskPath.c_str(), newDiskPath.c_str());
        // Revert the name in JSON
        m_actionDataFile["name"] = oldName;
        saveValues();
        return false;
    }

    printf("DebugActionWindow: Renamed action from '%s' to '%s'\n", oldName.c_str(), newName.c_str());

    // Update internal path
    m_actionFolderPath = newFolderPath;

    // Reload the action data from new location
    std::string actionFilePath = m_actionFolderPath + "/action.json";
    m_actionDataFile.load(actionFilePath);
    loadValues();

    // Update window title
    m_title = "Action Editor: " + newName;

    return true;
}
