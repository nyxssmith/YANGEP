#pragma once
#include "DebugWindow.h"

// Forward declarations
class AnimatedDataCharacterNavMeshPlayer;
class LevelV1;

class DebugPlayerInfoWindow : public DebugWindow
{
public:
    DebugPlayerInfoWindow(const std::string &title,
                          const AnimatedDataCharacterNavMeshPlayer &player,
                          const LevelV1 &level);
    virtual ~DebugPlayerInfoWindow() = default;
    void render() override;

private:
    const AnimatedDataCharacterNavMeshPlayer &m_player;
    const LevelV1 &m_level;
};
