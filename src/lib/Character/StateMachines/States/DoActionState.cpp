#include "DoActionState.h"
#include <cstdio>
#include "AnimatedDataCharacterNavMeshAgent.h"

DoActionState::DoActionState()
    : State(), action("")
{
}

DoActionState::~DoActionState()
{
}

void DoActionState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'action' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("action"))
    {
        if (defaults["action"].is_string())
        {
            action = defaults["action"].get<std::string>();
            // printf("DoActionState: Loaded action: '%s'\n", action.c_str());
        }
    }
}

void DoActionState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }
    // TODO tell agent to do the action specified by 'action'
    printf("DoActionState: Performing action '%s'\n", action.c_str());
    bool actionFound = false;
    // this->getAgent()->doAction(action);
    std::vector<Action> &agent_actions = getAgent()->getActions();
    for (auto &a : agent_actions)
    {
        if (a.getpath().find(action) != std::string::npos)
        {
            a.doAction();
            actionFound = true;
            break;
        }
    }
    if (!actionFound)
    {
        printf("DoActionState: Action '%s' not do-able by agent\n", action.c_str());
    }
    // Mark as complete
    setIsRunning(false);
}

const std::string &DoActionState::getAction() const
{
    return action;
}
