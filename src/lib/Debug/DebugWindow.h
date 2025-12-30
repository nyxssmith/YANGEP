#pragma once
#include <cute.h>
#include <dcimgui.h>
#include <string>

class DebugWindow
{
public:
    DebugWindow(const std::string &title);
    virtual ~DebugWindow() = default;
    virtual void render();

    bool isShown() const { return m_show; }

protected:
    std::string m_title;
    bool m_show;
};
