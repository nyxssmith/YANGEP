#include "DebugCharacterInfoWindow.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
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
        }
        else
        {
            ImGui_Text("No controller");
        }

        ImGui_Unindent();
    }

    ImGui_End();
}

bool DebugCharacterInfoWindow::isTracking(const AnimatedDataCharacter *character) const
{
    return m_character == character;
}

AnimatedDataCharacter *DebugCharacterInfoWindow::getCharacter() const
{
    return m_character;
}
