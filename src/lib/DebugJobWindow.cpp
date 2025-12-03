#include "DebugJobWindow.h"
#include "JobSystem.h"
#include <dcimgui.h>
#include <stdio.h>

DebugJobWindow::DebugJobWindow(const std::string &title)
    : m_title(title), m_totalJobsSubmitted(0), m_jobsCompletedLastFrame(0),
      m_jobsRunningLastFrame(0), m_showDetails(false)
{
}

void DebugJobWindow::render()
{
    if (!JobSystem::isInitialized())
    {
        ImGui_Begin(m_title.c_str(), nullptr, 0);
        ImGui_Text("Job System: NOT INITIALIZED");
        ImGui_End();
        return;
    }

    ImGui_Begin(m_title.c_str(), nullptr, 0);

    // Job system status header
    ImGui_Text("Job System Status");
    ImGui_Separator();

    // Worker thread count and pending jobs
    int workerCount = JobSystem::getWorkerCount();
    int pendingJobs = JobSystem::getPendingJobCount();
    ImGui_Text("Worker Threads: %d", workerCount);
    ImGui_Text("Pending Jobs: %d", pendingJobs);

    // System status
    if (JobSystem::isInitialized())
    {
        ImGui_TextColored(ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, "Status: RUNNING");
    }
    else
    {
        ImGui_TextColored(ImVec4{1.0f, 0.0f, 0.0f, 1.0f}, "Status: STOPPED");
    }

    ImGui_Separator();

    // Worker sections
    ImGui_Text("Workers:");

    auto workerInfo = JobSystem::getWorkerInfo();
    for (const auto &worker : workerInfo)
    {
        // Worker header with collapsing section
        char workerLabel[64];
        snprintf(workerLabel, sizeof(workerLabel), "Worker %d", worker.workerId);

        if (ImGui_CollapsingHeader(workerLabel, ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Label
            ImGui_Text("  Label: %s", worker.label.c_str());

            // Pending jobs in queue
            ImGui_Text("  Queued Jobs: %d", worker.pendingJobCount);

            // Status
            if (worker.isRunning)
            {
                ImGui_TextColored(ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, "  Status: BUSY");
                ImGui_Text("  Job: %s", worker.currentJobName.c_str());
            }
            else
            {
                ImGui_TextColored(ImVec4{0.5f, 0.5f, 0.5f, 1.0f}, "  Status: IDLE");
            }
        }
    }

    ImGui_Separator();

    // Show details toggle
    ImGui_Checkbox("Show API Info", &m_showDetails);

    if (m_showDetails)
    {
        ImGui_Separator();
        ImGui_Text("API Information:");
        ImGui_BulletText("submitJob(work, name) - Queue a task");
        ImGui_BulletText("kick() - Start jobs (non-blocking)");
        ImGui_BulletText("kickAndWait() - Start and wait");
    }

    ImGui_End();
}

const std::string &DebugJobWindow::getTitle() const
{
    return m_title;
}

void DebugJobWindow::setTitle(const std::string &title)
{
    m_title = title;
}
