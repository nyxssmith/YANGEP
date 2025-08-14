#include "DebugWindow.h"
#include <stdio.h>

DebugWindow::DebugWindow(const std::string &title)
    : m_title(title), m_show(true)
{
}

void DebugWindow::render()
{
    if (m_show)
    {
        igBegin(m_title.c_str(), &m_show, 0);
        igText("Debug Window: %s", m_title.c_str());
        if (igButton("Press me!", (ImVec2){0, 0}))
        {
            printf("Debug button clicked!\n");
        }
        static char buffer[256] = "Debug input...";
        igInputText("Debug Input", buffer, sizeof(buffer), 0, NULL, NULL);
        static float debug_float = 0.0f;
        igSliderFloat("Debug Float", &debug_float, 0, 10, "%.3f", 0);
        igEnd();
    }
}