#include "JumpState.h"
#include <cstdio>

JumpState::JumpState()
    : State(), targetStateIndex(-1)
{
}

JumpState::~JumpState()
{
}

void JumpState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'target_state_index' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("to"))
    {
        if (defaults["to"].is_number_integer())
        {
            targetStateIndex = defaults["to"].get<int>();
            // printf("JumpState: Loaded target state index: %d\n", targetStateIndex);
        }
    }
}

void JumpState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Print the target state index to stdout
    printf("Jumping to state index: %d\n", targetStateIndex);

    // Mark as complete
    setIsRunning(false);
}
