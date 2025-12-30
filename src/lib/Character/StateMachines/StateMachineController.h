#ifndef STATE_MACHINE_CONTROLLER_H
#define STATE_MACHINE_CONTROLLER_H

#include "StateMachine.h"
#include <vector>
#include <string>

class StateMachineController
{
public:
    StateMachineController();
    ~StateMachineController();

    // Add a state machine to the list
    void addStateMachine(StateMachine &&stateMachine);

    // Get the list of state machines
    const std::vector<StateMachine> &getStateMachines() const;

    // Get the current state machine name
    const std::string &getCurrentStateMachineName() const;

    // Set current state machine by name
    bool setCurrentStateMachine(const std::string &name);

    // Set current state machine by index
    bool setCurrentStateMachineByIndex(size_t index);

    // Get pointer to current state machine
    StateMachine *getCurrentStateMachine();

    // Get pointer to current state machine (const version)
    const StateMachine *getCurrentStateMachine() const;

private:
    std::vector<StateMachine> stateMachineList;
    std::string currentStateMachineName;
    StateMachine *currentStateMachine;

    // Update the current state machine pointer based on current name
    void updateCurrentStateMachinePointer();
};

#endif // STATE_MACHINE_CONTROLLER_H
