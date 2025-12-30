#include "PrintState.h"
#include <cstdio>

PrintState::PrintState()
    : State(), to_print("")
{
}

PrintState::~PrintState()
{
}

void PrintState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'to_print' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("to_print"))
    {
        if (defaults["to_print"].is_string())
        {
            to_print = defaults["to_print"].get<std::string>();
            printf("PrintState: Loaded print text: '%s'\n", to_print.c_str());
        }
    }
}

void PrintState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }

    // Print the text to stdout
    printf("%s\n", to_print.c_str());

    // Mark as complete
    setIsRunning(false);
}

const std::string &PrintState::getToPrint() const
{
    return to_print;
}
