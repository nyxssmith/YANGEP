#include "DebugSceneSwitcherWindow.h"
#include "DebugCharacterInfoWindow.h"
#include <algorithm>
#include <filesystem>
#include <imgui.h>

DebugSceneSwitcherWindow::DebugSceneSwitcherWindow(const std::string &title, SceneManager &sceneManager)
    : DebugWindow(title), m_sceneManager(sceneManager)
{
    const std::filesystem::path scenesDirectory("assets/DataFiles/Scenes");
    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator(scenesDirectory, error))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            m_scenes.push_back(entry.path().stem().string());
        }
    }
    std::sort(m_scenes.begin(), m_scenes.end());
}

void DebugSceneSwitcherWindow::render()
{
    if (!m_show)
    {
        return;
    }

    if (ImGui::Begin(m_title.c_str(), &m_show))
    {
        for (const auto &scene : m_scenes)
        {
            // Push a unique ID for this row so ImGui doesn't confuse the "Play"/"Edit" buttons
            ImGui::PushID(scene.c_str());

            // 1. Scene Name Label
            ImGui::Text("%s", scene.c_str());

            // 2. Play Button
            ImGui::SameLine(200.0f); // Adjust this float to align the buttons nicely in a column
            if (ImGui::Button("Play"))
            {
                // Separate code before doing the current action
                // ... do play prep work here ...
                // reload window config from disk
                m_sceneManager.ReloadWindowConfig();
                m_sceneManager.SetDebugVariablesFromWindowConfig();
                m_requestedScene = scene;
            }

            // 3. Edit Button
            ImGui::SameLine();
            if (ImGui::Button("Edit"))
            {
                // Separate code before doing the current action
                // ... do edit prep work here ...
                m_sceneManager.clickToCreateEntity = true;
                m_sceneManager.clickToInspectCharacter = true;
                m_sceneManager.debugHighlightTileUnderCursor = true;
                m_requestedScene = scene;
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
}

std::string DebugSceneSwitcherWindow::takeRequestedScene()
{
    std::string requested = std::move(m_requestedScene);
    m_requestedScene.clear();
    return requested;
}