#pragma once
#include "DebugWindow.h"

class DebugPrintControlWindow : public DebugWindow
{
public:
    DebugPrintControlWindow(const std::string &title);
    virtual ~DebugPrintControlWindow() = default;
    void render() override;

private:
    bool m_allChannelsEnabled;
};
