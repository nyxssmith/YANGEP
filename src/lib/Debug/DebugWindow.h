#pragma once
#include <string>
#include "DebugWindowList.h"

class DebugWindow
{
public:
    DebugWindow(const std::string &title);
    virtual ~DebugWindow() = default;
    virtual void render();

    bool isShown() const { return m_show; }
    void setDebugWindowList(DebugWindowList *debugWindowList) { m_debugWindowList = debugWindowList; }

protected:
    std::string m_title;
    bool m_show;
    float m_debugWindowScale;
    DebugWindowList *m_debugWindowList;
};
