#include "StateLibrary.h"
#include "States/WaitState.h"
#include "States/PrintState.h"
#include "States/WanderNewPositionState.h"
#include "States/MoveToPositionState.h"
#include "States/GoToTilePositionState.h"
#include "States/SetRegisterState.h"
#include "States/SayInBubbleState.h"
#include "States/DoActionState.h"
#include "States/DoActionFromCoordinatorState.h"
#include "States/GetNewPositionFromCoordinatorState.h"
#include "States/IfState.h"
#include "States/EndIfState.h"
#include "States/GetDistanceFromHomeState.h"
#include "States/GetDistanceFromPlayerState.h"
#include "States/GetEntitiesInBoxState.h"
#include "States/JumpState.h"
#include "States/FaceDirectionState.h"
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
    // waits a number of seconds before transitioning to the next state
    // Register WaitState
    registerState("wait", []() -> std::unique_ptr<State>
                  { return std::make_unique<WaitState>(); });

    // prints a message to the console before transitioning to the next state
    // Register PrintState
    registerState("print", []() -> std::unique_ptr<State>
                  { return std::make_unique<PrintState>(); });

    // implements GetNewPath and moves on once that is called
    // Register WanderNewPositionState
    registerState("wander_new_position", []() -> std::unique_ptr<State>
                  { return std::make_unique<WanderNewPositionState>(); });

    // Register MoveToPositionState
    // state exits when the character reaches the target position and doesnt have a navmesh path to follow
    registerState("move_to_position", []() -> std::unique_ptr<State>
                  { return std::make_unique<MoveToPositionState>(); });

    // Register GoToTilePositionState
    // takes in tile x and y and then needs move_to_position to actually move the character after that
    registerState("go_to_tile_position", []() -> std::unique_ptr<State>
                  { return std::make_unique<GoToTilePositionState>(); });
    // Register SetRegisterState
    registerState("set", []() -> std::unique_ptr<State>
                  { return std::make_unique<SetRegisterState>(); });

    // Register SayInBubbleState
    registerState("say_in_bubble", []() -> std::unique_ptr<State>
                  { return std::make_unique<SayInBubbleState>(); });
    // Register FaceDirectionState
    registerState("face_direction", []() -> std::unique_ptr<State>
                  { return std::make_unique<FaceDirectionState>(); });
    // Register DoActionState
    registerState("do_action", []() -> std::unique_ptr<State>
                  { return std::make_unique<DoActionState>(); });

    // Register DoActionFromCoordinatorState
    registerState("do_action_from_coordinator", []() -> std::unique_ptr<State>
                  { return std::make_unique<DoActionFromCoordinatorState>(); });

    // Register GetNewPositionFromCoordinatorState
    registerState("get_new_position_from_coordinator", []() -> std::unique_ptr<State>
                  { return std::make_unique<GetNewPositionFromCoordinatorState>(); });

    // Register IfState
    registerState("if", []() -> std::unique_ptr<State>
                  { return std::make_unique<IfState>(); });

    // Register EndIfState
    registerState("endif", []() -> std::unique_ptr<State>
                  { return std::make_unique<EndIfState>(); });

    // Register GetDistanceFromHomeState
    registerState("get_distance_from_home", []() -> std::unique_ptr<State>
                  { return std::make_unique<GetDistanceFromHomeState>(); });

    // Register GetDistanceFromPlayerState
    registerState("get_distance_from_player", []() -> std::unique_ptr<State>
                  { return std::make_unique<GetDistanceFromPlayerState>(); });

    // Register GetEntitiesInBoxState
    registerState("get_entities_in_box", []() -> std::unique_ptr<State>
                  { return std::make_unique<GetEntitiesInBoxState>(); });

    // Register JumpState
    // input is index of state to jump to in the state machine's list of states
    registerState("jump", []() -> std::unique_ptr<State>
                  { return std::make_unique<JumpState>(); });
}
