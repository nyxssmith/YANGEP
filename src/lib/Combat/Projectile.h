#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "AnimatedDataCharacterNavMeshAgent.h"
#include <string>
#include <vector>

struct ProjectileData
{
    float speed;        // speed
    float angle;        // angle where 0 is forward based on facing direction
    std::string entity; // entity name that makes up the projectile
};

struct ProjectilesData
{
    std::vector<ProjectileData> projectiles;
};

class Projectile : public AnimatedDataCharacterNavMeshAgent
{
public:
    Projectile();
    ~Projectile() override = default;
    bool init(const std::string &folderPath) override;
    void update(float dt, v2 moveVector);
    // Projectile specific methods would go here
    // For now, we will just expose the base class functionality.
    void setMoveVector(v2 mv) { moveVector = mv; }

private:
    // Projectile specific members, if any
    v2 moveVector;
};

#endif // PROJECTILE_H