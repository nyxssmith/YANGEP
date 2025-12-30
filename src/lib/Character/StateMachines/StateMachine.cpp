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

    // Load states from json data if it exists
    if (contains("states") && (*this)["states"].is_array())
    {
        const auto &statesArray = (*this)["states"];
        for (const auto &stateJson : statesArray)
        {
            // Each state should have a "name" field (the state type)
            if (!stateJson.contains("name") || !stateJson["name"].is_string())
            {
                printf("StateMachine '%s': Skipping state with missing or invalid 'name' field\n", name.c_str());
                continue;
            }

            std::string stateName = stateJson["name"];

            // Create the state from the library
            std::unique_ptr<State> state = stateLibrary.createState(stateName);

            if (!state)
            {
                printf("StateMachine '%s': Failed to create state '%s'\n", name.c_str(), stateName.c_str());
                continue;
            }

            // If the state has "inputs", use them to initialize the state
            if (stateJson.contains("inputs") && stateJson["inputs"].is_object())
            {
                DataFile inputs;
                *static_cast<nlohmann::json *>(&inputs) = stateJson["inputs"];
                state->setDefaultValues(inputs);
            }

            states.push_back(std::move(state));
            printf("StateMachine '%s': Added state '%s'\n", name.c_str(), stateName.c_str());
        }
    }
}
