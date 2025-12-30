#include "MoveToPositionState.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include <cstdio>

MoveToPositionState::MoveToPositionState()
    : State()
{
}

MoveToPositionState::~MoveToPositionState()
{
}

void MoveToPositionState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Currently no inputs needed for this state
}

void MoveToPositionState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Get the agent
    AnimatedDataCharacterNavMeshAgent *agent = getAgent();
    if (!agent)
    {
        printf("MoveToPositionState: No agent set, marking as complete\n");
        setIsRunning(false);
        return;
    }

    // Check if the current navigation path is null (path is complete or no path)
    auto currentPath = agent->getCurrentNavMeshPath();
    if (!currentPath || !currentPath->isValid() || currentPath->isComplete())
    {
        printf("MoveToPositionState: Path is complete or invalid, marking state as complete\n");
        setIsRunning(false);
    }
}
