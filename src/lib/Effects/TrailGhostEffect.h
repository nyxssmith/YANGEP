#ifndef TRAIL_GHOST_EFFECT_H
#define TRAIL_GHOST_EFFECT_H

#include "IVisualEffect.h"
#include "IGhostTrailEffect.h"
#include <deque>

class TrailGhostEffect : public IVisualEffect, public IGhostTrailEffect
{
public:
	explicit TrailGhostEffect(CF_Shader ghostShader);

	// IVisualEffect
	void trigger(int flashes, float totalDuration, float maxIntensity) override;
	void update(float dt) override;
	void beginDraw() override;
	void endDraw() override;
	bool isActive() const override;

	// IGhostTrailEffect
	void updateSubjectPosition(v2 position) override;
	int getGhostCount() const override;
	v2 getGhostPosition(int index) const override;
	float getGhostAlpha(int index) const override;
	CF_Shader getGhostShader() const override;
	float getGhostStrength(int index) const override;

private:
	bool m_active;
	float m_elapsed;
	float m_totalDuration;
	float m_baseAlpha;
	int m_maxGhosts;
	float m_recordInterval;   // seconds between position samples
	float m_recordTimer;
	float m_alphaDecay;       // per-ghost alpha falloff
	float m_strengthDecay;    // per-ghost shader strength falloff

	CF_Shader m_ghostShader;
	std::deque<v2> m_positions; // oldest at front, newest at back
	v2 m_lastPushed;
	bool m_hasLast;
};

#endif // TRAIL_GHOST_EFFECT_H


