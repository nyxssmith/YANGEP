#include "Projectile.h"
#include "LevelV1.h"
Projectile::Projectile()
{
}

bool Projectile::init(const std::string &folderPath)
{
    // only init the base class, its not a full crab or in need of more
    return AnimatedDataCharacter::init(folderPath);
}

void Projectile::update(float dt, v2 ignoredMoveVector)
{
    // do regular update for things like animation and navmesh checks
    AnimatedDataCharacterNavMeshAgent::update(dt, moveVector);
    // Add your custom projectile update logic here

    // check if the projectile is on or off navmesh
    // TODO ^
    // Check if this character is inside any active action hitbox in warmup phase
    std::vector<AnimatedDataCharacter *> agentsInArea = level->getAgentsInArea(characterHitbox->getBoxes(currentDirection, position),
                                                                               characterHitbox->getBoundingBox(currentDirection, position), this);
    if (agentsInArea.empty())
    {
        // exit early
        return;
    }
    // for each agent in the area, apply the action
    for (AnimatedDataCharacter *agent : agentsInArea)
    {
        // if the agent is the sender, skip it
        if (agent == sender)
        {
            continue;
        }
        // Apply the action to each agent
        action->doDamage(std::vector<AnimatedDataCharacter *>{agent});
    }
}

// printf("Inside action hitbox: %d\n", insideActionHitbox);
