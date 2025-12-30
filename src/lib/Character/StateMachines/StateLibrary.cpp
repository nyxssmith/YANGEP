#include "StateLibrary.h"
#include "States/WaitState.h"
#include "States/PrintState.h"
#include "States/WanderNewPositionState.h"
#include "States/MoveToPositionState.h"
#include <cstdio>

StateLibrary::StateLibrary()
{
    // Initialize with built-in states by default
    initializeBuiltInStates();
}

StateLibrary::~StateLibrary()
{
}

void StateLibrary::registerState(const std::string &name, std::function<std::unique_ptr<State>()> factory)
{
    stateFactories[name] = factory;
}

std::unique_ptr<State> StateLibrary::createState(const std::string &name) const
{
    auto it = stateFactories.find(name);
    if (it != stateFactories.end())
    {
        return it->second();
    }

    printf("StateLibrary: State '%s' not found in library\n", name.c_str());
    return nullptr;
}

std::unique_ptr<State> StateLibrary::createState(const std::string &name, const DataFile &defaultValues) const
{
    auto state = createState(name);
    if (state)
    {
        state->setDefaultValues(defaultValues);
    }
    return state;
}

bool StateLibrary::hasState(const std::string &name) const
{
    return stateFactories.find(name) != stateFactories.end();
}

std::vector<std::string> StateLibrary::getRegisteredStateNames() const
{
    std::vector<std::string> names;
    for (const auto &pair : stateFactories)
    {
        names.push_back(pair.first);
    }
    return names;
}

void StateLibrary::initializeBuiltInStates()
{
    // Register WaitState
    registerState("wait", []() -> std::unique_ptr<State>
                  { return std::make_unique<WaitState>(); });

    // Register PrintState
    registerState("print", []() -> std::unique_ptr<State>
                  { return std::make_unique<PrintState>(); });

    // Register WanderNewPositionState
    registerState("wander_new_position", []() -> std::unique_ptr<State>
                  { return std::make_unique<WanderNewPositionState>(); });

    // Register MoveToPositionState
    registerState("move_to_position", []() -> std::unique_ptr<State>
                  { return std::make_unique<MoveToPositionState>(); });
}
