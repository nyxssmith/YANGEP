#include "StateMachineController.h"
#include <cstdio>

StateMachineController::StateMachineController()
    : currentStateMachineName(""), currentStateMachine(nullptr)
{
}

StateMachineController::~StateMachineController()
{
}

void StateMachineController::addStateMachine(StateMachine &&stateMachine)
{
    std::string name = stateMachine.getName();
    stateMachineList.push_back(std::move(stateMachine));

    // If this is the first state machine, set it as current
    if (stateMachineList.size() == 1)
    {
        currentStateMachineName = name;
        currentStateMachine = &stateMachineList[0];
    }
}

const std::vector<StateMachine> &StateMachineController::getStateMachines() const
{
    return stateMachineList;
}

const std::string &StateMachineController::getCurrentStateMachineName() const
{
    return currentStateMachineName;
}

bool StateMachineController::setCurrentStateMachine(const std::string &name)
{
    for (size_t i = 0; i < stateMachineList.size(); i++)
    {
        if (stateMachineList[i].getName() == name)
        {
            currentStateMachineName = name;
            currentStateMachine = &stateMachineList[i];

            // Reset the state machine to its initial state
            currentStateMachine->reset();

            return true;
        }
    }

    printf("StateMachineController: State machine with name '%s' not found\n", name.c_str());
    return false;
}

bool StateMachineController::setCurrentStateMachineByIndex(size_t index)
{
    if (index < stateMachineList.size())
    {
        currentStateMachineName = stateMachineList[index].getName();
        currentStateMachine = &stateMachineList[index];

        // Reset the state machine to its initial state
        currentStateMachine->reset();

        return true;
    }

    printf("StateMachineController: Index %zu out of range (size: %zu)\n", index, stateMachineList.size());
    return false;
}

StateMachine *StateMachineController::getCurrentStateMachine()
{
    return currentStateMachine;
}

const StateMachine *StateMachineController::getCurrentStateMachine() const
{
    return currentStateMachine;
}

void StateMachineController::updateCurrentStateMachinePointer()
{
    for (size_t i = 0; i < stateMachineList.size(); i++)
    {
        if (stateMachineList[i].getName() == currentStateMachineName)
        {
            currentStateMachine = &stateMachineList[i];
            return;
        }
    }

    // If not found and list is not empty, default to first
    if (!stateMachineList.empty())
    {
        currentStateMachine = &stateMachineList[0];
        currentStateMachineName = stateMachineList[0].getName();
    }
    else
    {
        currentStateMachine = nullptr;
        currentStateMachineName = "";
    }
}
