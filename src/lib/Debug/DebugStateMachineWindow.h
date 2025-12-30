#pragma once
#include "DebugWindow.h"
#include "DebugStateWindow.h"
#include <vector>
#include <memory>

// Forward declarations
class StateMachine;

class DebugStateMachineWindow : public DebugWindow
{
public:
    DebugStateMachineWindow(const std::string &title, StateMachine *stateMachine);
    virtual ~DebugStateMachineWindow() = default;
    void render() override;

    // Check if this window is tracking the given state machine
    bool isTracking(const StateMachine *stateMachine) const;

    // Get the tracked state machine pointer
    StateMachine *getStateMachine() const;

private:
    StateMachine *m_stateMachine;                                  // Non-owning pointer to the state machine
    std::vector<std::unique_ptr<DebugStateWindow>> m_stateWindows; // State debug windows
};
