#include "DebugCharacterInfoWindow.h"
#include "DebugStateMachineWindow.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "StateMachine.h"
#include "StateMachineController.h"
#include "LevelV1.h"
#include <cute.h>

DebugCharacterInfoWindow::DebugCharacterInfoWindow(const std::string &title,
                                                   AnimatedDataCharacter *character,
                                                   const LevelV1 &level)
    : DebugWindow(title), m_character(character), m_level(level)
{
}

void DebugCharacterInfoWindow::render()
{
    if (!m_show || !m_character)
    {
        return;
    }

    ImGui_Begin(m_title.c_str(), &m_show, 0);

    // Display datafile path
    ImGui_Text("Datafile:");
    ImGui_Indent();
    ImGui_TextWrapped("%s", m_character->getDataFilePath().c_str());
    ImGui_Unindent();

    ImGui_Separator();

    // Get character world position
    CF_V2 worldPos = m_character->getPosition();

    // Get tile dimensions from level
    int tileWidth = m_level.getTileWidth();
    int tileHeight = m_level.getTileHeight();

    // Calculate tile coordinates from world position
    float tileX = worldPos.x / static_cast<float>(tileWidth);
    float tileY = worldPos.y / static_cast<float>(tileHeight);

    // Display world coordinates
    ImGui_Text("World Position:");
    ImGui_Indent();
    ImGui_Text("X: %.2f", worldPos.x);
    ImGui_Text("Y: %.2f", worldPos.y);
    ImGui_Unindent();

    ImGui_Separator();

    // Display tile coordinates
    ImGui_Text("Tile Position:");
    ImGui_Indent();
    ImGui_Text("X: %.2f (tile %d)", tileX, static_cast<int>(tileX));
    ImGui_Text("Y: %.2f (tile %d)", tileY, static_cast<int>(tileY));
    ImGui_Unindent();

    ImGui_Separator();

    // Display current direction
    ImGui_Text("Direction: %d", static_cast<int>(m_character->getCurrentDirection()));

    ImGui_Separator();

    // Display action state
    ImGui_Text("Action State:");
    ImGui_Indent();
    ImGui_Text("Doing Action: %s", m_character->getIsDoingAction() ? "Yes" : "No");
    if (m_character->getActiveAction())
    {
        ImGui_Text("Active Action: Present");
    }
    else
    {
        ImGui_Text("Active Action: None");
    }
    ImGui_Unindent();

    ImGui_Separator();

    // Display and control Stage of Life
    ImGui_Text("Stage of Life:");
    ImGui_Indent();

    // Get current stage of life
    StageOfLife currentStage = m_character->getStageOfLife();

    // Create combo box with stage options (null-separated string)
    int currentStageIndex = static_cast<int>(currentStage);

    if (ImGui_Combo("##StageOfLife", &currentStageIndex, "Alive\0Dying\0Dead\0"))
    {
        // User changed the selection, update the character's stage
        StageOfLife newStage = static_cast<StageOfLife>(currentStageIndex);
        m_character->setStageOfLife(newStage);

        // Close window if character was set to Dead
        if (newStage == StageOfLife::Dead)
        {
            m_show = false;
        }
    }

    ImGui_Unindent();

    ImGui_Separator();

    // Display state machine info if this is a NavMeshAgent
    AnimatedDataCharacterNavMeshAgent *agent = dynamic_cast<AnimatedDataCharacterNavMeshAgent *>(m_character);
    if (agent)
    {
        ImGui_Text("State Machine:");
        ImGui_Indent();

        StateMachineController *controller = agent->getStateMachineController();
        if (controller)
        {
            const std::string &currentName = controller->getCurrentStateMachineName();
            if (!currentName.empty())
            {
                ImGui_Text("Current: %s", currentName.c_str());
            }
            else
            {
                ImGui_Text("Current: None");
            }

            const auto &machines = controller->getStateMachines();
            ImGui_Text("Total Machines: %zu", machines.size());

            // Show buttons for each state machine
            ImGui_Separator();
            ImGui_Text("State Machines:");
            for (size_t i = 0; i < machines.size(); ++i)
            {
                // machines is a vector of StateMachine objects, not pointers
                StateMachine *machine = const_cast<StateMachine *>(&machines[i]);

                const std::string &machineName = machine->getName();
                ImGui_Text("  %s", machineName.c_str());
                ImGui_SameLine();

                // Button to set this as the current state machine
                char setCurrentLabel[128];
                snprintf(setCurrentLabel, sizeof(setCurrentLabel), "Set Current##machine_%zu", i);
                if (ImGui_ButtonEx(setCurrentLabel, (ImVec2){0, 0}))
                {
                    controller->setCurrentStateMachine(machineName);
                }
                ImGui_SameLine();

                // Button to open debug window for this state machine
                char buttonLabel[128];
                snprintf(buttonLabel, sizeof(buttonLabel), "Debug##machine_%zu", i);
                if (ImGui_ButtonEx(buttonLabel, (ImVec2){0, 0}))
                {
                    // Check if we already have a window for this state machine
                    bool found = false;
                    for (auto &window : m_stateMachineWindows)
                    {
                        if (window->isTracking(machine))
                        {
                            found = true;
                            break;
                        }
                    }

                    // If not, create a new one
                    if (!found)
                    {
                        std::string windowTitle = "State Machine: " + machineName;
                        m_stateMachineWindows.push_back(
                            std::make_unique<DebugStateMachineWindow>(windowTitle, machine));
                    }
                }
            }
        }
        else
        {
            ImGui_Text("No controller");
        }

        ImGui_Unindent();
    }

    ImGui_End();

    // Render all state machine windows
    for (auto &window : m_stateMachineWindows)
    {
        window->render();
    }

    // Clean up closed windows
    m_stateMachineWindows.erase(
        std::remove_if(m_stateMachineWindows.begin(), m_stateMachineWindows.end(),
                       [](const std::unique_ptr<DebugStateMachineWindow> &w)
                       { return !w->isShown(); }),
        m_stateMachineWindows.end());
}

bool DebugCharacterInfoWindow::isTracking(const AnimatedDataCharacter *character) const
{
    return m_character == character;
}

AnimatedDataCharacter *DebugCharacterInfoWindow::getCharacter() const
{
    return m_character;
}
