#include "State.h"

State::State()
    : defaultValues(), isRunning(false)
{
}

State::State(const DataFile &defaultValues)
    : defaultValues(defaultValues), isRunning(false)
{
}

State::State(const std::string &datafilePath)
    : defaultValues(), isRunning(false)
{
    if (defaultValues.load(datafilePath))
    {
        initFromJson();
    }
}

State::State(const nlohmann::json &jsonData)
    : defaultValues(), isRunning(false)
{
    // Copy json data into defaultValues
    *static_cast<nlohmann::json *>(&defaultValues) = jsonData;
    initFromJson();
}

State::~State()
{
}

void State::update(float dt)
{
    // Base implementation does nothing
    // Override in derived classes
}

bool State::getIsRunning() const
{
    return isRunning;
}

void State::setIsRunning(bool running)
{
    isRunning = running;
}

const DataFile &State::getDefaultValues() const
{
    return defaultValues;
}

void State::setDefaultValues(const DataFile &values)
{
    defaultValues = values;
    // Re-initialize from the new values
    initFromJson();
}

void State::initFromJson()
{
    // Base implementation does nothing
    // Override in derived classes if needed
}
