#include "EffectFactory.h"
#include "RedFlashEffect.h"
#include "GreenFlashEffect.h"
#include "TrailGhostEffect.h"
#include "ShaderRegistry.h"

std::unique_ptr<IVisualEffect> EffectFactory::makeEffect(const std::string& name)
{
	if (name == "red")
	{
		return std::unique_ptr<IVisualEffect>(new RedFlashEffect(ShaderRegistry::get("red")));
	}
	if (name == "green")
	{
		return std::unique_ptr<IVisualEffect>(new GreenFlashEffect(ShaderRegistry::get("green")));
	}
	if (name == "trail")
	{
		return std::unique_ptr<IVisualEffect>(new TrailGhostEffect(ShaderRegistry::get("trail")));
	}
	return nullptr;
}


