#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "DataFile.h"
#include "StateLibrary.h"
#include "State.h"
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

class StateMachine : public DataFile
{
public:
    StateMachine();
    explicit StateMachine(const std::string &datafilePath);
    explicit StateMachine(const nlohmann::json &jsonData);
    ~StateMachine();

    // Delete copy operations (contains unique_ptrs)
    StateMachine(const StateMachine &) = delete;
    StateMachine &operator=(const StateMachine &) = delete;

    // Move operations
    StateMachine(StateMachine &&) = default;
    StateMachine &operator=(StateMachine &&) = default;

    // Get the name of this state machine
    const std::string &getName() const;

    // Set the name of this state machine
    void setName(const std::string &newName);

    // Get the state library
    StateLibrary *getStateLibrary();
    const StateLibrary *getStateLibrary() const;

    // Get the list of states
    const std::vector<std::unique_ptr<State>> &getStates() const;
    std::vector<std::unique_ptr<State>> &getStates();

    // Get the current state
    State *getCurrentState();
    const State *getCurrentState() const;

    // Set the current state by name
    // Returns true if state was found and set, false otherwise
    bool setCurrentState(const std::string &stateName);

    // Update the current state and check if it's done running
    void update(float dt);

    // Get the loop counter (how many times the state machine has cycled)
    int getLoopCounter() const;

private:
    std::string name;
    StateLibrary stateLibrary;
    std::vector<std::unique_ptr<State>> states;
    int currentStateIndex; // -1 if no state is active
    int loopCounter;       // Increments each time the state machine cycles back to first state

    // Common initialization from json data
    void initFromJson();
};

#endif // STATE_MACHINE_H
