#pragma once
#include "DebugWindow.h"

// Forward declarations
class AnimatedDataCharacter;
class LevelV1;

class DebugCharacterInfoWindow : public DebugWindow
{
public:
    DebugCharacterInfoWindow(const std::string &title,
                             AnimatedDataCharacter *character,
                             const LevelV1 &level);
    virtual ~DebugCharacterInfoWindow() = default;
    void render() override;

    // Check if this window is tracking the given character
    bool isTracking(const AnimatedDataCharacter *character) const;

    // Get the tracked character pointer
    AnimatedDataCharacter *getCharacter() const;

private:
    AnimatedDataCharacter *m_character; // Non-owning pointer to the character
    const LevelV1 &m_level;
};
