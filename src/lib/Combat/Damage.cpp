#include "Damage.h"

Damage::Damage()
    : value(0)
{
}

Damage::Damage(int damageValue)
    : value(damageValue)
{
}

Damage::~Damage()
{
}

int Damage::getDamageTo(int multiplier, AnimatedDataCharacter *character)
{
    int damage = value * multiplier;
    // todo extend this for damage types and do stuff with health etc
    return damage;
}
