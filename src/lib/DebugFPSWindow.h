#pragma once
#include "DebugWindow.h"

class DebugFPSWindow : public DebugWindow
{
public:
    DebugFPSWindow(const std::string &title);
    virtual ~DebugFPSWindow() = default;
    void render() override;
};
