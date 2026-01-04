#ifndef I_VISUAL_EFFECT_H
#define I_VISUAL_EFFECT_H

class IVisualEffect
{
public:
	virtual ~IVisualEffect() {}

	// Configure and start the effect.
	virtual void trigger(int flashes, float totalDuration, float maxIntensity) = 0;

	// Advance timers.
	virtual void update(float dt) = 0;

	// Apply effect before drawing the subject.
	virtual void beginDraw() = 0;

	// Restore state after drawing the subject.
	virtual void endDraw() = 0;

	// Whether the effect is currently active.
	virtual bool isActive() const = 0;
};

#endif // I_VISUAL_EFFECT_H


