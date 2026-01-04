#ifndef DISSOLVE_EFFECT_H
#define DISSOLVE_EFFECT_H

#include "VisualEffectBase.h"
#include <cute.h>

using namespace Cute;

class DissolveEffect : public VisualEffectBase
{
public:
	explicit DissolveEffect(CF_Shader shader);

	// IVisualEffect
	void trigger(int flashes, float totalDuration, float maxIntensity) override;
	void update(float dt) override;
	void beginDraw() override;
	void endDraw() override;
	bool isActive() const override;
	void setOnComplete(std::function<void()> callback) override;

private:
	bool m_active;
	float m_elapsed;
	float m_totalDuration;
	float m_edgeWidth;     // in 0..0.2 typical
	CF_Shader m_shader;
	bool m_shaderPushed;
	std::function<void()> m_onComplete;
};

#endif // DISSOLVE_EFFECT_H


