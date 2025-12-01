#include "Behavior.h"

// Constructor
Behavior::Behavior()
{
}

// Destructor
Behavior::~Behavior()
{
}

// Base implementation does nothing - returns an invalid path
std::shared_ptr<NavMeshPath> Behavior::GetNewPath(NavMesh &navmesh, CF_V2 currentPosition, int radius)
{
    // Does nothing - returns empty/invalid path
    return std::make_shared<NavMeshPath>();
}
