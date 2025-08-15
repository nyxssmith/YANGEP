#pragma once
#include <cute.h>
#include <cimgui.h>
#include <string>

class DebugWindow
{
public:
    DebugWindow(const std::string &title);
    virtual ~DebugWindow() = default;
    virtual void render();

protected:
    std::string m_title;
    bool m_show;
};
