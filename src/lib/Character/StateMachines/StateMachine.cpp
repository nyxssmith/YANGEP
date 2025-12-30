#include "StateMachine.h"

StateMachine::StateMachine()
    : DataFile(), name("")
{
}

StateMachine::StateMachine(const std::string &datafilePath)
    : DataFile(), name("")
{
    if (load(datafilePath))
    {
        initFromJson();
    }
}

StateMachine::StateMachine(const nlohmann::json &jsonData)
    : DataFile(), name("")
{
    // Copy json data into this DataFile (which is a json object)
    *static_cast<nlohmann::json *>(this) = jsonData;
    initFromJson();
}

StateMachine::~StateMachine()
{
}

const std::string &StateMachine::getName() const
{
    return name;
}

void StateMachine::setName(const std::string &newName)
{
    name = newName;
}

StateLibrary *StateMachine::getStateLibrary()
{
    return &stateLibrary;
}

const StateLibrary *StateMachine::getStateLibrary() const
{
    return &stateLibrary;
}

void StateMachine::initFromJson()
{
    // Load name from json data if it exists
    if (contains("name"))
    {
        name = (*this)["name"].get<std::string>();
    }
}
