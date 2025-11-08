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
NavMeshPath Behavior::GetNewPath(const NavMesh &navmesh, CF_V2 currentPosition, int radius)
{
    NavMeshPath path;
    // Does nothing - returns empty/invalid path
    return path;
}
