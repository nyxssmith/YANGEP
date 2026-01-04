#ifndef DAMAGE_H
#define DAMAGE_H

class Damage
{
public:
    Damage();
    Damage(float damageValue);
    ~Damage();

    float value;

    void doDamage(float multiplier);

private:
};

#endif // DAMAGE_H
