#pragma once
#include "DebugWindow.h"
#include <vector>
#include <string>
#include <chrono>

struct ProfileSection
{
    std::string name;
    double duration_ms;
};

class DebugFPSWindow : public DebugWindow
{
public:
    DebugFPSWindow(const std::string &title);
    virtual ~DebugFPSWindow() = default;
    void render() override;

    // Profiling methods
    void beginFrame();
    void markSection(const std::string &sectionName);
    void endFrame();

private:
    std::chrono::high_resolution_clock::time_point m_frameStart;
    std::chrono::high_resolution_clock::time_point m_lastMark;
    std::vector<ProfileSection> m_sections;
    double m_totalFrameTime_ms;
};
