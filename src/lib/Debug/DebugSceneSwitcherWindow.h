#pragma once

#include "DebugWindow.h"
#include <string>
#include <vector>
#include "SceneManager.h"

class DebugSceneSwitcherWindow : public DebugWindow
{
public:
    DebugSceneSwitcherWindow(const std::string &title, SceneManager &sceneManager);
    void render() override;
    std::string takeRequestedScene();
    SceneManager &m_sceneManager;

private:
    std::vector<std::string> m_scenes;
    std::string m_requestedScene;
};