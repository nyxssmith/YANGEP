#ifndef STATE_H
#define STATE_H

#include "DataFile.h"
#include <string>
#include <nlohmann/json.hpp>

class State
{
public:
    State();
    explicit State(const DataFile &defaultValues);
    explicit State(const std::string &datafilePath);
    explicit State(const nlohmann::json &jsonData);
    virtual ~State();

    // Update the state
    virtual void update(float dt);

    // Get/Set running state
    bool getIsRunning() const;
    void setIsRunning(bool running);

    // Get the default values datafile
    const DataFile &getDefaultValues() const;
    void setDefaultValues(const DataFile &values);

private:
    DataFile defaultValues;
    bool isRunning;

    // Common initialization from json data
    void initFromJson();
};

#endif // STATE_H
