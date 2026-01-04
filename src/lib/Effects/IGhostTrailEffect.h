#ifndef I_GHOST_TRAIL_EFFECT_H
#define I_GHOST_TRAIL_EFFECT_H

#include <cute.h>
#include <vector>

using namespace Cute;

class IGhostTrailEffect
{
public:
	virtual ~IGhostTrailEffect() {}

	// Supply the subject's current world position each frame.
	virtual void updateSubjectPosition(v2 position) = 0;

	// Number of ghost instances to draw (oldest first recommended).
	virtual int getGhostCount() const = 0;

	// World-space position for the i-th ghost (0 = oldest).
	virtual v2 getGhostPosition(int index) const = 0;

	// Alpha multiplier [0..1] for the i-th ghost (0 = oldest).
	virtual float getGhostAlpha(int index) const = 0;

	// Optional shader to use for ghost draws. Can be {0} for none.
	virtual CF_Shader getGhostShader() const = 0;

	// Optional per-ghost strength parameter for the ghost shader.
	// If the shader expects a 'strength' float, return the value here.
	virtual float getGhostStrength(int index) const = 0;
};

#endif // I_GHOST_TRAIL_EFFECT_H


