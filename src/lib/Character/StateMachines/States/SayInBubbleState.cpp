#include "SayInBubbleState.h"
#include <cstdio>

SayInBubbleState::SayInBubbleState()
    : State(), to_say("")
{
}

SayInBubbleState::~SayInBubbleState()
{
}

void SayInBubbleState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'to_say' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("to_say"))
    {
        if (defaults["to_say"].is_string())
        {
            to_say = defaults["to_say"].get<std::string>();
            // printf("SayInBubbleState: Loaded text to say: '%s'\n", to_say.c_str());
        }
    }
}

void SayInBubbleState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Print the text to stdout
    printf("%s\n", to_say.c_str());
    // TODO make speechbubble appear with the text

    // Mark as complete
    setIsRunning(false);
}

const std::string &SayInBubbleState::getToSay() const
{
    return to_say;
}
