#include "DebugFPSWindow.h"
#include <cute.h>

DebugFPSWindow::DebugFPSWindow(const std::string &title)
    : DebugWindow(title)
{
}

void DebugFPSWindow::render()
{
    if (m_show)
    {
        ImGui_Begin(m_title.c_str(), &m_show, 0);

        // Get FPS from cute framework
        float fps = cf_app_get_smoothed_framerate();
        float raw_fps = cf_app_get_framerate();

        // Display FPS metrics
        ImGui_Text("FPS (smoothed): %.1f", fps);
        ImGui_Text("FPS (raw): %.1f", raw_fps);
        ImGui_Separator();
        ImGui_Text("Frame time: %.3f ms", 1000.0f / fps);

        ImGui_End();
    }
}
