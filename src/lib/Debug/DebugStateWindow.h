#pragma once
#include "DebugWindow.h"

// Forward declarations
class State;

class DebugStateWindow : public DebugWindow
{
public:
    DebugStateWindow(const std::string &title, State *state);
    virtual ~DebugStateWindow() = default;
    void render() override;

    // Check if this window is tracking the given state
    bool isTracking(const State *state) const;

    // Get the tracked state pointer
    State *getState() const;

private:
    State *m_state; // Non-owning pointer to the state
};
