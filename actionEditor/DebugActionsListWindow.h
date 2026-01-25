#pragma once
#include "DebugWindow.h"
#include "DebugActionWindow.h"
#include <string>
#include <vector>
#include <memory>

class DebugActionsListWindow : public DebugWindow
{
public:
    DebugActionsListWindow(const std::string &title, std::unique_ptr<DebugActionWindow> &actionWindowRef);
    void render() override;

private:
    std::vector<std::string> m_actionFolders;
    std::unique_ptr<DebugActionWindow> &m_actionWindowRef;
    char m_newActionNameBuffer[64];

    void refreshActionsList();
    bool createNewAction(const std::string &actionName);
};
