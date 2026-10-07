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
    AnimatedDataCharacterNavMeshAgent::update(dt, moveVector);
    // Add your custom projectile update logic here

    // Default color is yellow for character hitbox
    CF_Color color = cf_make_color_rgb(255, 255, 0); // Yellow

    // Check if this character is inside any active action hitbox in warmup phase
    bool insideActionHitbox = false;
    if (level)
    {
        CF_Aabb characterBox = characterHitbox->getBoundingBox(currentDirection, position);
        insideActionHitbox = level->isCharacterInActionHitbox(this, characterBox);
    }
    printf("Inside action hitbox: %d\n", insideActionHitbox);
}
