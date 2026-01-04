#ifndef EFFECT_FACTORY_H
#define EFFECT_FACTORY_H

#include <memory>
#include <string>
#include "IVisualEffect.h"

class EffectFactory
{
public:
	static std::unique_ptr<IVisualEffect> makeEffect(const std::string& name);
};

#endif // EFFECT_FACTORY_H


