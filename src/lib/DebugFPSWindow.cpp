#include "DebugFPSWindow.h"
#include <cute.h>

DebugFPSWindow::DebugFPSWindow(const std::string &title)
    : DebugWindow(title), m_totalFrameTime_ms(0.0)
{
}

void DebugFPSWindow::beginFrame()
{
    m_frameStart = std::chrono::high_resolution_clock::now();
    m_lastMark = m_frameStart;
    m_sections.clear();
}

void DebugFPSWindow::markSection(const std::string &sectionName)
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastMark);
    double duration_ms = duration.count() / 1000.0;

    m_sections.push_back({sectionName, duration_ms});
    m_lastMark = now;
}

void DebugFPSWindow::endFrame()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_frameStart);
    m_totalFrameTime_ms = totalDuration.count() / 1000.0;
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

        // Display profiled frame time
        ImGui_Text("Frame time: %.3f ms", m_totalFrameTime_ms);

        // Display profiled sections
        if (!m_sections.empty())
        {
            ImGui_Indent();
            for (const auto &section : m_sections)
            {
                ImGui_Text("- %s: %.3f ms", section.name.c_str(), section.duration_ms);
            }
            ImGui_Unindent();
        }

        ImGui_End();
    }
}
