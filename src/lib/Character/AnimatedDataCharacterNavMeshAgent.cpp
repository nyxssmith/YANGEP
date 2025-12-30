#include "AnimatedDataCharacterNavMeshAgent.h"
#include "JobSystem.h"
#include "DataFile.h"
#include "StateMachine.h"
#include <cute.h>
#include <cstdio>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshAgent::AnimatedDataCharacterNavMeshAgent()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1),
      currentNavMeshPath(nullptr),
      backgroundJobRunning(false), backgroundJobComplete(false),
      backgroundMoveVector(cf_v2(0.0f, 0.0f))
{
}

// Destructor
AnimatedDataCharacterNavMeshAgent::~AnimatedDataCharacterNavMeshAgent()
{
    // We don't own the navmesh, so we don't delete it
}

// Override init to also load state machines
bool AnimatedDataCharacterNavMeshAgent::init(const std::string &folderPath)
{
    // Call parent init first
    if (!AnimatedDataCharacter::init(folderPath))
    {
        return false;
    }

    // Load state machines from the same folder
    loadStateMachinesFromFolder(folderPath);

    return true;
}

// Set the navmesh this agent is operating on
void AnimatedDataCharacterNavMeshAgent::setNavMesh(NavMesh *navmesh)
{
    this->navmesh = navmesh;
    updateCurrentPolygon();
}

// Get the navmesh this agent is on
NavMesh *AnimatedDataCharacterNavMeshAgent::getNavMesh() const
{
    return navmesh;
}

// Check if agent has a navmesh assigned
bool AnimatedDataCharacterNavMeshAgent::hasNavMesh() const
{
    return navmesh != nullptr;
}

// Get the current polygon the agent is on
int AnimatedDataCharacterNavMeshAgent::getCurrentPolygon() const
{
    return currentPolygon;
}

// Update the agent's current polygon based on position
void AnimatedDataCharacterNavMeshAgent::updateCurrentPolygon()
{
    if (!navmesh)
    {
        currentPolygon = -1;
        return;
    }

    // Get the agent's position from parent class
    v2 agentPosition = getPosition();
    CF_V2 cutePosition = cf_v2(agentPosition.x, agentPosition.y);

    currentPolygon = navmesh->findPolygonAt(cutePosition);
}

// Check if agent is currently on walkable area
bool AnimatedDataCharacterNavMeshAgent::isOnWalkableArea() const
{
    if (!navmesh)
    {
        return false;
    }

    // Get the agent's position from parent class
    v2 agentPosition = getPosition();
    CF_V2 cutePosition = cf_v2(agentPosition.x, agentPosition.y);

    return navmesh->isWalkable(cutePosition);
}

// Override update to track navmesh position
void AnimatedDataCharacterNavMeshAgent::update(float dt, v2 moveVector)
{

    // call the current state update
    StateMachine *currentStateMachine = stateMachineController.getCurrentStateMachine();
    if (currentStateMachine)
    {
        State *currentState = currentStateMachine->getCurrentState();
        if (currentState)
        {
            currentState->update(dt);
        }
    }

    // Call parent update
    AnimatedDataCharacter::update(dt, moveVector);

    // Update our current polygon after movement
    if (navmesh)
    {
        updateCurrentPolygon();
    }
}

// Background update - submits AI/pathfinding calculations to job system
bool AnimatedDataCharacterNavMeshAgent::backgroundUpdate(float dt, bool isOnScreen)
{
    // Check if a job is already running
    if (backgroundJobRunning.load())
    {
        return false; // Job already in progress
    }

    // Mark job as running and not complete
    backgroundJobRunning.store(true);
    backgroundJobComplete.store(false);

    // Submit the calculation job to the job system with a name
    JobSystem::submitJob([this, dt, isOnScreen]()
                         {
        if (isOnScreen)
        {
            this->OnScreenBackgroundUpdateJob(dt);
        }
        else
        {
            this->OffScreenBackgroundUpdateJob(dt);
        }
        
        // Mark job as complete
        this->backgroundJobComplete.store(true);
        this->backgroundJobRunning.store(false); },
                         "Agent AI Update");

    return true; // Job submitted successfully
}

// Check if background job is complete
bool AnimatedDataCharacterNavMeshAgent::isBackgroundUpdateComplete() const
{
    return backgroundJobComplete.load();
}

// Get the result of the background update
v2 AnimatedDataCharacterNavMeshAgent::getBackgroundMoveVector() const
{
    return backgroundMoveVector;
}

// Get the current navigation path
std::shared_ptr<NavMeshPath> AnimatedDataCharacterNavMeshAgent::getCurrentNavMeshPath()
{
    return currentNavMeshPath;
}

// Get the current navigation path (const version)
std::shared_ptr<const NavMeshPath> AnimatedDataCharacterNavMeshAgent::getCurrentNavMeshPath() const
{
    return currentNavMeshPath;
}

// Set the current navigation path
void AnimatedDataCharacterNavMeshAgent::setCurrentNavMeshPath(std::shared_ptr<NavMeshPath> path)
{
    currentNavMeshPath = path;
}

// Clear the current navigation path
void AnimatedDataCharacterNavMeshAgent::clearCurrentNavMeshPath()
{
    currentNavMeshPath = nullptr;
}

// Get the wander behavior
WanderBehavior *AnimatedDataCharacterNavMeshAgent::getWanderBehavior()
{
    return &wanderBehavior;
}

// Get the wander behavior (const version)
const WanderBehavior *AnimatedDataCharacterNavMeshAgent::getWanderBehavior() const
{
    return &wanderBehavior;
}

// Get the state machine controller
StateMachineController *AnimatedDataCharacterNavMeshAgent::getStateMachineController()
{
    return &stateMachineController;
}

// Get the state machine controller (const version)
const StateMachineController *AnimatedDataCharacterNavMeshAgent::getStateMachineController() const
{
    return &stateMachineController;
}

// Background update job for on-screen agents (more detailed AI)
void AnimatedDataCharacterNavMeshAgent::OnScreenBackgroundUpdateJob(float dt)
{
    // Check if navmesh is available
    if (navmesh == nullptr)
    {
        backgroundMoveVector = cf_v2(0.0f, 0.0f);
        return;
    }

    // Get current position
    v2 agentPosition = getPosition();
    CF_V2 currentPosition = cf_v2(agentPosition.x, agentPosition.y);

    // if has a path that is valid and not complete
    if (currentNavMeshPath && currentNavMeshPath->isValid() && !currentNavMeshPath->isComplete())
    {
        // if current position is at current waypoint of the path
        if (currentNavMeshPath->isAtCurrentWaypoint(currentPosition))
        {
            // call getnext for the path
            currentNavMeshPath->getNext();
        }

        // Get the current waypoint (without advancing)
        CF_V2 *nextWaypoint = currentNavMeshPath->getCurrent();

        // if nextwaypoint is null
        if (nextWaypoint == nullptr)
        {
            // if current path is not null, mark it as complete
            if (currentNavMeshPath)
            {
                currentNavMeshPath->markComplete();
            }
            // get new path from current state
            StateMachine *currentStateMachine = stateMachineController.getCurrentStateMachine();
            if (currentStateMachine)
            {
                State *currentState = currentStateMachine->getCurrentState();
                if (currentState)
                {
                    currentNavMeshPath = currentState->GetNewPath(*navmesh, currentPosition);
                }
            }

            if (!currentNavMeshPath || !currentNavMeshPath->isValid())
            {
                backgroundMoveVector = cf_v2(0.0f, 0.0f);
            }
            return;
        }

        // set a new backgroundMoveVector toward next waypoint
        float dirX = nextWaypoint->x - currentPosition.x;
        float dirY = nextWaypoint->y - currentPosition.y;

        // Normalize and scale to movement speed
        float length = sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0f)
        {
            dirX = (dirX / length) * 100.0f; // Scale to speed
            dirY = (dirY / length) * 100.0f;
        }

        backgroundMoveVector = cf_v2(dirX, dirY);
    }
    // else
    else
    {
        // get new path from current state
        StateMachine *currentStateMachine = stateMachineController.getCurrentStateMachine();
        if (currentStateMachine)
        {
            State *currentState = currentStateMachine->getCurrentState();
            if (currentState)
            {
                currentNavMeshPath = currentState->GetNewPath(*navmesh, currentPosition);
            }
        }

        backgroundMoveVector = cf_v2(0.0f, 0.0f);
    }
}

// Background update job for off-screen agents (simplified AI)
void AnimatedDataCharacterNavMeshAgent::OffScreenBackgroundUpdateJob(float dt)
{
    OnScreenBackgroundUpdateJob(dt);
    // TODO: Implement simplified AI for off-screen agents
    // This might include:
    // - Simpler pathfinding or path following
    // - Basic behavior updates
    // - Minimal state changes
    // - Skip expensive calculations that won't be visible
}

// Background AI calculation (runs in worker thread)
void AnimatedDataCharacterNavMeshAgent::calculateMoveVector(float dt)
{
}

// Load state machines from a folder containing state_machines.json
bool AnimatedDataCharacterNavMeshAgent::loadStateMachinesFromFolder(const std::string &folderPath)
{
    // Construct the path to state_machines.json
    std::string stateMachinesPath = folderPath + "/state_machines.json";

    // Create a DataFile to load the state machines configuration
    DataFile stateMachinesData;
    if (!stateMachinesData.load(stateMachinesPath))
    {
        printf("AnimatedDataCharacterNavMeshAgent: Failed to load state_machines.json from '%s'\n", folderPath.c_str());
        return false;
    }

    // Check for required keys
    if (!stateMachinesData.contains("state_machines") || !stateMachinesData["state_machines"].is_array())
    {
        printf("AnimatedDataCharacterNavMeshAgent: state_machines.json missing 'state_machines' array\n");
        return false;
    }

    if (!stateMachinesData.contains("default_state_machine"))
    {
        printf("AnimatedDataCharacterNavMeshAgent: state_machines.json missing 'default_state_machine' key\n");
        return false;
    }

    printf("AnimatedDataCharacterNavMeshAgent: Loaded state_machines.json from '%s'\n", folderPath.c_str());

    // Iterate through each state machine in the array
    const auto &stateMachinesArray = stateMachinesData["state_machines"];
    for (const auto &stateMachineJson : stateMachinesArray)
    {
        // Check if this is a string instead of a JSON object
        if (stateMachineJson.is_string())
        {
            // The string is a filename - load the state machine from file
            std::string stateMachineName = stateMachineJson.get<std::string>();
            std::string stateMachinePath = "assets/DataFiles/StateMachines/" + stateMachineName + ".json";

            // Load the state machine file
            DataFile stateMachineFile;
            if (!stateMachineFile.load(stateMachinePath))
            {
                printf("  - WARNING: Failed to load state machine file '%s'\n", stateMachinePath.c_str());
                continue;
            }

            // Create a StateMachine from the loaded file (DataFile inherits from nlohmann::json)
            StateMachine stateMachine(stateMachineFile);

            printf("  - Adding state machine from file: '%s'\n", stateMachine.getName().c_str());

            // Add it to the controller (move ownership)
            stateMachineController.addStateMachine(std::move(stateMachine));
            continue;
        }

        // Create a StateMachine from the JSON blob
        StateMachine stateMachine(stateMachineJson);

        printf("  - Adding state machine: '%s'\n", stateMachine.getName().c_str());

        // Add it to the controller (move ownership)
        stateMachineController.addStateMachine(std::move(stateMachine));
    }

    // Set the default state machine
    std::string defaultStateMachineName = stateMachinesData["default_state_machine"];
    if (stateMachineController.setCurrentStateMachine(defaultStateMachineName))
    {
        printf("  - Set default state machine to: '%s'\n", defaultStateMachineName.c_str());
    }
    else
    {
        printf("  - WARNING: Failed to set default state machine '%s'\n", defaultStateMachineName.c_str());
    }

    return true;
}
