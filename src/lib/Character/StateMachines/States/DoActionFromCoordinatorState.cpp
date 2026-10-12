#include "DoActionFromCoordinatorState.h"
#include <cstdio>

DoActionFromCoordinatorState::DoActionFromCoordinatorState()
    : State(), action("")
{
}

DoActionFromCoordinatorState::~DoActionFromCoordinatorState()
{
}

void DoActionFromCoordinatorState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // TODO get the level coordinator from agent->level->coordinator
}

void DoActionFromCoordinatorState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }
    // TODO tell agent to do the action specified by the coordinator

    // Mark as complete
    setIsRunning(false);
}

const std::string &DoActionFromCoordinatorState::getAction() const
{
    return action;
}
