#include "DebugSceneSwitcherWindow.h"
#include <algorithm>
#include <filesystem>
#include <imgui.h>

DebugSceneSwitcherWindow::DebugSceneSwitcherWindow(const std::string &title)
    : DebugWindow(title)
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
            if (ImGui::Button(scene.c_str()))
            {
                m_requestedScene = scene;
            }
        }
    }
    ImGui::End();
}

std::string DebugSceneSwitcherWindow::takeRequestedScene()
{
    std::string requestedScene = std::move(m_requestedScene);
    m_requestedScene.clear();
    return requestedScene;
}