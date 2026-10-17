#ifndef DAMAGE_H
#define DAMAGE_H

class AnimatedDataCharacter;

class Damage
{
public:
    Damage();
    Damage(int damageValue);
    ~Damage();

    int value;

    int getDamageTo(int multiplier, AnimatedDataCharacter *character);

private:
};

#endif // DAMAGE_H
