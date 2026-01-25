#include "DebugActionsListWindow.h"
#include <imgui.h>
#include <stdio.h>
#include <cute.h>
#include <filesystem>
#include <fstream>
#include "DataFile.h"

namespace fs = std::filesystem;

DebugActionsListWindow::DebugActionsListWindow(const std::string &title, std::unique_ptr<DebugActionWindow> &actionWindowRef)
    : DebugWindow(title), m_actionWindowRef(actionWindowRef)
{
    m_show = true; // Always open by default
    memset(m_newActionNameBuffer, 0, sizeof(m_newActionNameBuffer));
    refreshActionsList();
}

void DebugActionsListWindow::refreshActionsList()
{
    m_actionFolders.clear();

    // Use Cute Framework's filesystem to list directories
    const char *actionsPath = "/assets/DataFiles/Actions";

    // Get list of files/folders in the Actions directory
    const char **entries = cf_fs_enumerate_directory(actionsPath);

    if (entries)
    {
        for (const char **i = entries; *i; ++i)
        {
            const char *name = *i;
            // Build full path to check if it's a directory
            std::string fullPath = std::string(actionsPath) + "/" + name;
            CF_Stat stat;
            if (!cf_is_error(cf_fs_stat(fullPath.c_str(), &stat)))
            {
                // Only include directories (actions are stored as folders)
                if (stat.type == CF_FILE_TYPE_DIRECTORY)
                {
                    m_actionFolders.push_back(name);
                }
            }
        }
        cf_fs_free_enumerated_directory(entries);
    }

    printf("DebugActionsListWindow: Found %zu action folders\n", m_actionFolders.size());
}

void DebugActionsListWindow::render()
{
    if (!m_show)
        return;

    ImGui::Begin(m_title.c_str(), &m_show, 0);

    ImGui::Text("Actions in DataFiles/Actions:");
    ImGui::Separator();

    // Create new action section
    ImGui::Text("Create New Action:");
    ImGui::InputText("##NewActionName", m_newActionNameBuffer, sizeof(m_newActionNameBuffer), 0);
    ImGui::SameLine();
    if (ImGui::Button("Create"))
    {
        std::string actionName = m_newActionNameBuffer;
        if (!actionName.empty())
        {
            if (createNewAction(actionName))
            {
                // Clear the input buffer
                memset(m_newActionNameBuffer, 0, sizeof(m_newActionNameBuffer));
                refreshActionsList();
            }
        }
        else
        {
            printf("DebugActionsListWindow: Action name cannot be empty\n");
        }
    }

    ImGui::Separator();

    // Refresh button
    if (ImGui::Button("Refresh List"))
    {
        refreshActionsList();
    }

    ImGui::Separator();

    // List all action folders with edit buttons
    for (const auto &folderName : m_actionFolders)
    {
        ImGui::Text("%s", folderName.c_str());
        ImGui::SameLine();

        // Create unique button ID
        std::string buttonLabel = "Edit##" + folderName;

        if (ImGui::Button(buttonLabel.c_str()))
        {
            // Close existing action window if any
            m_actionWindowRef.reset();

            // Create new action window for this action
            std::string actionPath = "/assets/DataFiles/Actions/" + folderName;
            m_actionWindowRef = std::make_unique<DebugActionWindow>("Action Editor: " + folderName, actionPath);

            printf("DebugActionsListWindow: Opened editor for action '%s'\n", folderName.c_str());
        }
    }

    if (m_actionFolders.empty())
    {
        ImGui::Text("No actions found. Click 'Refresh List' to update.");
    }

    ImGui::End();
}

bool DebugActionsListWindow::createNewAction(const std::string &actionName)
{
    // Create the folder path for VFS access
    std::string folderPath = "/assets/DataFiles/Actions/" + actionName;

    // Create the real filesystem path (relative to executable)
    std::string realPath = "assets/DataFiles/Actions/" + actionName;

    // Check if folder already exists in VFS
    CF_Stat stat;
    if (!cf_is_error(cf_fs_stat(folderPath.c_str(), &stat)))
    {
        printf("DebugActionsListWindow: Action '%s' already exists\n", actionName.c_str());
        return false;
    }

    // Create the directory in the real filesystem
    try
    {
        fs::create_directories(realPath);
        printf("DebugActionsListWindow: Created directory %s\n", realPath.c_str());
    }
    catch (const std::exception &e)
    {
        printf("DebugActionsListWindow: Failed to create directory %s: %s\n", realPath.c_str(), e.what());
        return false;
    }

    // Create default action.json directly to filesystem
    nlohmann::json actionData;
    actionData["name"] = actionName;
    actionData["version"] = "1.0.0";
    actionData["description"] = "A new action";
    actionData["warmup"] = 100.0f;
    actionData["cooldown"] = 500.0f;
    actionData["global_cooldown"] = 0.0f;

    std::string actionFilePath = realPath + "/action.json";
    try
    {
        std::ofstream actionFile(actionFilePath);
        if (!actionFile.is_open())
        {
            printf("DebugActionsListWindow: Failed to open %s for writing\n", actionFilePath.c_str());
            return false;
        }
        actionFile << actionData.dump(2);
        actionFile.close();
        printf("DebugActionsListWindow: Created %s\n", actionFilePath.c_str());
    }
    catch (const std::exception &e)
    {
        printf("DebugActionsListWindow: Failed to write action.json: %s\n", e.what());
        return false;
    }

    // Create default hitbox.json directly to filesystem
    nlohmann::json hitboxData;
    nlohmann::json tiles = nlohmann::json::array();
    nlohmann::json defaultTile;
    defaultTile["x"] = 1;
    defaultTile["y"] = 0;
    defaultTile["damage_modifier"] = 1.0f;
    tiles.push_back(defaultTile);
    hitboxData["tiles"] = tiles;

    std::string hitboxFilePath = realPath + "/hitbox.json";
    try
    {
        std::ofstream hitboxFile(hitboxFilePath);
        if (!hitboxFile.is_open())
        {
            printf("DebugActionsListWindow: Failed to open %s for writing\n", hitboxFilePath.c_str());
            return false;
        }
        hitboxFile << hitboxData.dump(2);
        hitboxFile.close();
        printf("DebugActionsListWindow: Created %s\n", hitboxFilePath.c_str());
    }
    catch (const std::exception &e)
    {
        printf("DebugActionsListWindow: Failed to write hitbox.json: %s\n", e.what());
        return false;
    }

    printf("DebugActionsListWindow: Successfully created new action '%s'\n", actionName.c_str());

    // Open the new action in the editor
    m_actionWindowRef.reset();
    m_actionWindowRef = std::make_unique<DebugActionWindow>("Action Editor: " + actionName, folderPath);

    return true;
}
