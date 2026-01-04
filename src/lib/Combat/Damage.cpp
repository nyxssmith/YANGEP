#include "Damage.h"

Damage::Damage()
    : value(0.0f)
{
}

Damage::Damage(float damageValue)
    : value(damageValue)
{
}

Damage::~Damage()
{
}

void Damage::doDamage(float multiplier)
{
    // Empty for now
    // todo extend this for damage types and do stuff with health etc
}
