#include "DebugWindow.h"
#include <imgui.h>
#include <stdio.h>

DebugWindow::DebugWindow(const std::string &title)
    : m_title(title), m_show(true)
{
}

void DebugWindow::render()
{
    if (m_show)
    {
        ImGui::Begin(m_title.c_str(), &m_show);
        ImGui::Text("Debug Window: %s", m_title.c_str());
        if (ImGui::Button("Press me!"))
        {
            printf("Debug button clicked!\n");
        }
        static char buffer[256] = "Debug input...";
        ImGui::InputText("Debug Input", buffer, sizeof(buffer));
        static float debug_float = 0.0f;
        ImGui::SliderFloat("Debug Float", &debug_float, 0, 10, "%.3f");
        ImGui::End();
    }
}