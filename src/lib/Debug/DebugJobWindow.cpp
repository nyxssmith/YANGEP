#include "DebugJobWindow.h"
#include "JobSystem.h"
#include <imgui.h>
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
        ImGui::Begin(m_title.c_str(), nullptr);
        ImGui::Text("Job System: NOT INITIALIZED");
        ImGui::End();
        return;
    }

    ImGui::Begin(m_title.c_str(), nullptr);

    // Job system status header
    ImGui::Text("Job System Status");
    ImGui::Separator();

    // Worker thread count and pending jobs
    int workerCount = JobSystem::getWorkerCount();
    int pendingJobs = JobSystem::getPendingJobCount();
    ImGui::Text("Worker Threads: %d", workerCount);
    ImGui::Text("Pending Jobs: %d", pendingJobs);

    // System status
    if (JobSystem::isInitialized())
    {
        ImGui::TextColored(ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, "Status: RUNNING");
    }
    else
    {
        ImGui::TextColored(ImVec4{1.0f, 0.0f, 0.0f, 1.0f}, "Status: STOPPED");
    }

    ImGui::Separator();

    // Worker sections
    ImGui::Text("Workers:");

    auto workerInfo = JobSystem::getWorkerInfo();
    for (const auto &worker : workerInfo)
    {
        // Worker header with collapsing section
        char workerLabel[64];
        snprintf(workerLabel, sizeof(workerLabel), "Worker %d", worker.workerId);

        if (ImGui::CollapsingHeader(workerLabel, ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Label
            ImGui::Text("  Label: %s", worker.label.c_str());

            // Pending jobs in queue
            ImGui::Text("  Queued Jobs: %d", worker.pendingJobCount);

            // Running jobs
            ImGui::Text("  Running Jobs: %d", worker.runningJobCount);

            // Status
            if (worker.runningJobCount > 0)
            {
                ImGui::TextColored(ImVec4{0.0f, 1.0f, 0.0f, 1.0f}, "  Status: BUSY");
            }
            else
            {
                ImGui::TextColored(ImVec4{0.5f, 0.5f, 0.5f, 1.0f}, "  Status: IDLE");
            }
        }
    }

    ImGui::Separator();

    // Show details toggle
    ImGui::Checkbox("Show API Info", &m_showDetails);

    if (m_showDetails)
    {
        ImGui::Separator();
        ImGui::Text("API Information:");
        ImGui::BulletText("submitJob(work, name) - Queue a task");
        ImGui::BulletText("kick() - Start jobs (non-blocking)");
        ImGui::BulletText("kickAndWait() - Start and wait");
    }

    ImGui::End();
}

const std::string &DebugJobWindow::getTitle() const
{
    return m_title;
}

void DebugJobWindow::setTitle(const std::string &title)
{
    m_title = title;
}
