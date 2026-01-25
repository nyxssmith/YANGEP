#pragma once
#include "DebugWindow.h"
#include <cute.h>

class DebugInputInfoWindow : public DebugWindow
{
public:
    DebugInputInfoWindow(const std::string &title);
    virtual ~DebugInputInfoWindow() = default;
    void render() override;

private:
    void renderKeyboardSection();
    void renderMouseSection();
    void renderJoystickSection();
    void renderMovementSection();
};
