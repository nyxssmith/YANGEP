#include "DebugFPSWindow.h"
#include <cute.h>
#include <imgui.h>

DebugFPSWindow::DebugFPSWindow(const std::string &title)
    : DebugWindow(title), m_totalFrameTime_ms(0.0), m_lowestFPSLast1000Frames(0.0f)
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

    // Track FPS history
    float currentFPS = cf_app_get_framerate();
    m_fpsHistory.push_back(currentFPS);

    // Keep only last 1000 frames
    if (m_fpsHistory.size() > MAX_FPS_HISTORY)
    {
        m_fpsHistory.pop_front();
    }

    // Calculate lowest FPS from history
    if (!m_fpsHistory.empty())
    {
        m_lowestFPSLast1000Frames = *std::min_element(m_fpsHistory.begin(), m_fpsHistory.end());
    }
}

void DebugFPSWindow::render()
{
    if (m_show)
    {
        ImGui::Begin(m_title.c_str(), &m_show);

        // Get FPS from cute framework
        float fps = cf_app_get_smoothed_framerate();
        float raw_fps = cf_app_get_framerate();

        // Display FPS metrics
        ImGui::Text("FPS (smoothed): %.1f", fps);
        ImGui::Text("FPS (raw): %.1f", raw_fps);
        ImGui::Text("Lowest FPS (last 1000 frames): %.1f", m_lowestFPSLast1000Frames);
        ImGui::Separator();

        // Display profiled frame time
        ImGui::Text("Frame time: %.3f ms", m_totalFrameTime_ms);

        // Display profiled sections
        if (!m_sections.empty())
        {
            ImGui::Indent();
            for (const auto &section : m_sections)
            {
                ImGui::Text("- %s: %.3f ms", section.name.c_str(), section.duration_ms);
            }
            ImGui::Unindent();
        }

        ImGui::End();
    }
}
