#pragma once

#include "DebugWindow.h"
#include <string>
#include <vector>

class DebugSceneSwitcherWindow : public DebugWindow
{
public:
    explicit DebugSceneSwitcherWindow(const std::string &title);
    void render() override;
    std::string takeRequestedScene();

private:
    std::vector<std::string> m_scenes;
    std::string m_requestedScene;
};