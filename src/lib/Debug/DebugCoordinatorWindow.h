#pragma once
#include "DebugWindow.h"

// Forward declarations
class Coordinator;
class AnimatedDataCharacter;
class LevelV1;

class DebugCoordinatorWindow : public DebugWindow
{
public:
    DebugCoordinatorWindow(const std::string &title,
                           Coordinator *coordinator,
                           const AnimatedDataCharacter *player,
                           const LevelV1 &level);
    virtual ~DebugCoordinatorWindow() = default;
    void render() override;

private:
    Coordinator *m_coordinator;            // Non-owning pointer to the coordinator
    const AnimatedDataCharacter *m_player; // Non-owning pointer to the player
    const LevelV1 &m_level;                // Reference to the level
};
