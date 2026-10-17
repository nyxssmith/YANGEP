#include "FaceDirectionState.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include <cstdio>

FaceDirectionState::FaceDirectionState()
    : State(), direction()
{
}

FaceDirectionState::~FaceDirectionState()
{
}

void FaceDirectionState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'direction' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("direction"))
    {
        if (defaults["direction"].is_string())
        {
            std::string direction_str = defaults["direction"].get<std::string>();
            if (direction_str == "up")
            {
                direction = Direction::UP;
            }
            else if (direction_str == "down")
            {
                direction = Direction::DOWN;
            }
            else if (direction_str == "left")
            {
                direction = Direction::LEFT;
            }
            else if (direction_str == "right")
            {
                direction = Direction::RIGHT;
            }
        }
    }
}

void FaceDirectionState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }
    printf("FaceDirectionState: Facing direction: %d\n", static_cast<int>(direction));
    getAgent()->setDirection(direction);
    // Mark as complete
    setIsRunning(false);
}
