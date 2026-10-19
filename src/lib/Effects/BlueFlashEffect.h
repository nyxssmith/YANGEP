#ifndef BLUE_FLASH_EFFECT_H
#define BLUE_FLASH_EFFECT_H

#include <cute.h>
#include "VisualEffectBase.h"

using namespace Cute;

class BlueFlashEffect : public VisualEffectBase
{
public:
	BlueFlashEffect(CF_Shader shader);

	void trigger(int flashes = 3, float totalDuration = 2.0f, float maxIntensity = 0.85f) override;
	void update(float dt) override;
	void beginDraw() override;
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

#endif // BLUE_FLASH_EFFECT_H
