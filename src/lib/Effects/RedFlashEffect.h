#ifndef RED_FLASH_EFFECT_H
#define RED_FLASH_EFFECT_H

#include <cute.h>
#include "VisualEffectBase.h"

using namespace Cute;

class RedFlashEffect : public VisualEffectBase
{
public:
	RedFlashEffect(CF_Shader shader);

	// Start a red flashing sequence.
	void trigger(int flashes = 3, float totalDuration = 2.0f, float maxIntensity = 0.85f) override;

	// Advance internal timers.
	void update(float dt) override;

	// Apply shader before drawing the subject.
	void beginDraw() override;

	// Pop shader after drawing the subject.
	void endDraw() override;

	bool isActive() const override;

private:
	float computeIntensity() const;

	bool m_active;
	float m_elapsed;
	int m_flashes;
	float m_totalDuration;
	float m_maxIntensity;
	CF_Shader m_shader;
	bool m_shaderPushed;
};

#endif // RED_FLASH_EFFECT_H


