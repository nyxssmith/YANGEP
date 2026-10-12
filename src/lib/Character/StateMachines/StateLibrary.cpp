#include "StateLibrary.h"
#include "States/WaitState.h"
#include "States/PrintState.h"
#include "States/WanderNewPositionState.h"
#include "States/MoveToPositionState.h"
#include "States/GoToTilePositionState.h"
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

    // planned states
    //  do_action input:action ; makes the character do that action if they can
    //  get_path_from_coordinator ; uses getnewpath to ask the coordinator where its attack the player position is, and gets a path there (also needs move_to_position after to move)
    //  do_action_from_coordinator ; makes the character do an action based on instructions from the coordinator if possible (default to 0th action in list)
    //  say_in_bubble ; makes the character display a speech bubble with the given text
    //  NEXT STATES REQUIRE REGISTERS TO BE ADDED
    // plan is to add registers for A, B, C where A is a bool, B is a float and C is a string, and their value can be used with $A, $B, $C respectively as state inputs
    //  set : input is {A:<new bool>, B:<new float>, C:<new string>} Sets the value of either A, B, or C to the specified new value
    // TODO add and handle the following states and implement todos in others
    //  if : input is {A:<bool>, B:<float>, C:<string>, operation:<string for > >= == != < <=} Compares the value of either A, B, or C based on the operation specified, if the comparison is true, the next state is executed, otherwise it skips to the next else state
    //  endif : takes no inputs, is used as a skip to here if if is false
    //  get_distance_from_player ; gets the distance from the player character and sets it in the B register
    //  get_distance_from_home ; gets the distance from the character's home position (where it spawned, TODO need to add this to be set when state machine starts up) and sets it in the B register
    //  jump : input is index of state to jump to in the state machine's list of states
}
