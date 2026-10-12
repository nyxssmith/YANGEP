#include "EndIfState.h"
#include <cstdio>

EndIfState::EndIfState()
    : State()
{
}

EndIfState::~EndIfState()
{
}

void EndIfState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // No specific initialization needed for EndIfState
}

void EndIfState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Mark as complete
    setIsRunning(false);
}
