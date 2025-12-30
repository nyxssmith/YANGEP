#ifndef STATE_H
#define STATE_H

#include "DataFile.h"
#include "NavMesh.h"
#include "NavMeshPath.h"
#include <string>
#include <nlohmann/json.hpp>
#include <memory>

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

    // Get a new navigation path based on the state
    // currentPosition: the current position of the agent
    // Returns a path through the navmesh (may be invalid if no path found)
    virtual std::shared_ptr<NavMeshPath> GetNewPath(NavMesh &navmesh, CF_V2 currentPosition);

    // Determine the direction the agent should face
    // currentDirection: the current direction the agent is facing
    // Returns the direction the agent should face
    virtual CF_V2 FaceDirection(CF_V2 currentDirection);

protected:
    // Common initialization from json data
    virtual void initFromJson();

private:
    DataFile defaultValues;
    bool isRunning;
};

#endif // STATE_H
