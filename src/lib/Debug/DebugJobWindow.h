#pragma once

#include <string>

// Debug window for displaying job system statistics and monitoring
class DebugJobWindow
{
public:
    DebugJobWindow(const std::string &title);
    ~DebugJobWindow() = default;

    // Render the debug window
    void render();

    // Get the window title
    const std::string &getTitle() const;

    // Set the window title
    void setTitle(const std::string &title);

private:
    std::string m_title;

    // Track statistics
    int m_totalJobsSubmitted;
    int m_jobsCompletedLastFrame;
    int m_jobsRunningLastFrame;

    // UI state
    bool m_showDetails;
};
