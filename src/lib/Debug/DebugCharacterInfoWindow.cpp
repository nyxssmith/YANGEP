#include "DebugCharacterInfoWindow.h"
#include "DebugStateMachineWindow.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "StateMachine.h"
#include "StateMachineController.h"
#include "LevelV1.h"
#include <cute.h>
#include <imgui.h>

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

    ImGui::Begin(m_title.c_str(), &m_show);

    // Display datafile path
    ImGui::Text("Datafile:");
    ImGui::Indent();
    ImGui::TextWrapped("%s", m_character->getDataFilePath().c_str());
    ImGui::Unindent();

    ImGui::Separator();

    // Get character world position
    CF_V2 worldPos = m_character->getPosition();

    // Get tile dimensions from level
    int tileWidth = m_level.getTileWidth();
    int tileHeight = m_level.getTileHeight();

    // Calculate tile coordinates from world position
    float tileX = worldPos.x / static_cast<float>(tileWidth);
    float tileY = worldPos.y / static_cast<float>(tileHeight);

    // Display world coordinates
    ImGui::Text("World Position:");
    ImGui::Indent();
    ImGui::Text("X: %.2f", worldPos.x);
    ImGui::Text("Y: %.2f", worldPos.y);
    ImGui::Unindent();

    ImGui::Separator();

    // Display tile coordinates
    ImGui::Text("Tile Position:");
    ImGui::Indent();
    ImGui::Text("X: %.2f (tile %d)", tileX, static_cast<int>(tileX));
    ImGui::Text("Y: %.2f (tile %d)", tileY, static_cast<int>(tileY));
    ImGui::Unindent();

    ImGui::Separator();

    // Display current direction
    ImGui::Text("Direction: %d", static_cast<int>(m_character->getCurrentDirection()));

    ImGui::Separator();

    // Display action state
    ImGui::Text("Action State:");
    ImGui::Indent();
    ImGui::Text("Doing Action: %s", m_character->getIsDoingAction() ? "Yes" : "No");
    if (m_character->getActiveAction())
    {
        ImGui::Text("Active Action: Present");
    }
    else
    {
        ImGui::Text("Active Action: None");
    }
    ImGui::Unindent();

    ImGui::Separator();

    // Display and control Stage of Life
    ImGui::Text("Stage of Life:");
    ImGui::Indent();

    // Get current stage of life
    StageOfLife currentStage = m_character->getStageOfLife();

    // Create combo box with stage options (null-separated string)
    int currentStageIndex = static_cast<int>(currentStage);

    const char* stageNames[] = {"Alive", "Dying", "Dead"};
    if (ImGui::Combo("##StageOfLife", &currentStageIndex, stageNames, 3))
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

    ImGui::Unindent();

    ImGui::Separator();

    // Display state machine info if this is a NavMeshAgent
    if (m_character != nullptr)
    {
        // TODO crashes if character is destroyed while window is open
        AnimatedDataCharacterNavMeshAgent *agent = dynamic_cast<AnimatedDataCharacterNavMeshAgent *>(m_character);
        if (agent)
        {
            ImGui::Text("State Machine:");
            ImGui::Indent();

            StateMachineController *controller = agent->getStateMachineController();
            if (controller)
            {
                const std::string &currentName = controller->getCurrentStateMachineName();
                if (!currentName.empty())
                {
                    ImGui::Text("Current: %s", currentName.c_str());
                }
                else
                {
                    ImGui::Text("Current: None");
                }

                const auto &machines = controller->getStateMachines();
                ImGui::Text("Total Machines: %zu", machines.size());

                // Show buttons for each state machine
                ImGui::Separator();
                ImGui::Text("State Machines:");
                for (size_t i = 0; i < machines.size(); ++i)
                {
                    // machines is a vector of StateMachine objects, not pointers
                    StateMachine *machine = const_cast<StateMachine *>(&machines[i]);

                    const std::string &machineName = machine->getName();
                    ImGui::Text("  %s", machineName.c_str());
                    ImGui::SameLine();

                    // Button to set this as the current state machine
                    char setCurrentLabel[128];
                    snprintf(setCurrentLabel, sizeof(setCurrentLabel), "Set Current##machine_%zu", i);
                    if (ImGui::Button(setCurrentLabel))
                    {
                        controller->setCurrentStateMachine(machineName);
                    }
                    ImGui::SameLine();

                    // Button to open debug window for this state machine
                    char buttonLabel[128];
                    snprintf(buttonLabel, sizeof(buttonLabel), "Debug##machine_%zu", i);
                    if (ImGui::Button(buttonLabel))
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
                ImGui::Text("No controller");
            }

            ImGui::Unindent();
        }
    }

    ImGui::End();

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
