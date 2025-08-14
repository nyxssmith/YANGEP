#pragma once
#include <cute.h>
#include <cimgui.h>
#include <string>

class DebugWindow
{
public:
    DebugWindow(const std::string &title);
    void render();

private:
    std::string m_title;
    bool m_show;
};
