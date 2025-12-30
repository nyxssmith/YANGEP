#include "DebugStateMachineWindow.h"
#include "DebugStateWindow.h"
#include "StateMachine.h"
#include "State.h"
#include "DataFile.h"
#include <stdio.h>

DebugStateMachineWindow::DebugStateMachineWindow(const std::string &title, StateMachine *stateMachine)
    : DebugWindow(title), m_stateMachine(stateMachine)
{
}

void DebugStateMachineWindow::render()
{
    if (!m_show || !m_stateMachine)
    {
        return;
    }

    ImGui_Begin(m_title.c_str(), &m_show, 0);

    // Display state machine name
    ImGui_Text("State Machine: %s", m_stateMachine->getName().c_str());
    ImGui_Text("Loop Counter: %d", m_stateMachine->getLoopCounter());
    ImGui_Separator();

    // Get current state
    State *currentState = m_stateMachine->getCurrentState();

    // Display all states
    ImGui_Text("States:");
    ImGui_Indent();

    const auto &states = m_stateMachine->getStates();
    if (states.empty())
    {
        ImGui_Text("(No states)");
    }
    else
    {
        for (size_t i = 0; i < states.size(); ++i)
        {
            State *state = states[i].get();
            if (!state)
            {
                continue;
            }

            // Get the state name from its default values
            const DataFile &stateData = state->getDefaultValues();
            std::string stateName = "Unknown";
            if (stateData.contains("name") && stateData["name"].is_string())
            {
                stateName = stateData["name"].get<std::string>();
            }

            // Mark current state
            if (state == currentState)
            {
                ImGui_Text("[CURRENT] %s", stateName.c_str());
                ImGui_SameLine();
            }
            else
            {
                ImGui_Text("          %s", stateName.c_str());
                ImGui_SameLine();
            }

            // Button to open debug window for this state
            char buttonLabel[128];
            snprintf(buttonLabel, sizeof(buttonLabel), "Debug##state_%zu", i);
            if (ImGui_ButtonEx(buttonLabel, (ImVec2){0, 0}))
            {
                // Check if we already have a window for this state
                bool found = false;
                for (auto &window : m_stateWindows)
                {
                    if (window->isTracking(state))
                    {
                        found = true;
                        break;
                    }
                }

                // If not, create a new one
                if (!found)
                {
                    std::string windowTitle = "State: " + stateName;
                    m_stateWindows.push_back(std::make_unique<DebugStateWindow>(windowTitle, state));
                }
            }
        }
    }

    ImGui_Unindent();

    ImGui_End();

    // Render all state windows
    for (auto &window : m_stateWindows)
    {
        window->render();
    }

    // Clean up closed windows
    m_stateWindows.erase(
        std::remove_if(m_stateWindows.begin(), m_stateWindows.end(),
                       [](const std::unique_ptr<DebugStateWindow> &w)
                       { return !w->isShown(); }),
        m_stateWindows.end());
}

bool DebugStateMachineWindow::isTracking(const StateMachine *stateMachine) const
{
    return m_stateMachine == stateMachine;
}

StateMachine *DebugStateMachineWindow::getStateMachine() const
{
    return m_stateMachine;
}
