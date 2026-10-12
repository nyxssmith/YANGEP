#include "DoActionState.h"
#include <cstdio>

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

    // Mark as complete
    setIsRunning(false);
}

const std::string &DoActionState::getAction() const
{
    return action;
}
