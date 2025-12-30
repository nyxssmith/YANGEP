#ifndef STATE_LIBRARY_H
#define STATE_LIBRARY_H

#include "State.h"
#include <string>
#include <memory>
#include <map>
#include <functional>

class StateLibrary
{
public:
    StateLibrary();
    ~StateLibrary();

    // Register a state type with a factory function
    void registerState(const std::string &name, std::function<std::unique_ptr<State>()> factory);

    // Create a state by name
    std::unique_ptr<State> createState(const std::string &name) const;

    // Create a state by name with default values
    std::unique_ptr<State> createState(const std::string &name, const DataFile &defaultValues) const;

    // Check if a state type is registered
    bool hasState(const std::string &name) const;

    // Get all registered state names
    std::vector<std::string> getRegisteredStateNames() const;

    // Initialize the library with built-in states
    void initializeBuiltInStates();

private:
    std::map<std::string, std::function<std::unique_ptr<State>()>> stateFactories;
};

#endif // STATE_LIBRARY_H
