#include "TrailGhostEffect.h"
#include <algorithm>

TrailGhostEffect::TrailGhostEffect(CF_Shader ghostShader)
	: m_active(false)
	, m_elapsed(0.0f)
	, m_totalDuration(1.0f)
	, m_baseAlpha(0.8f)
	, m_maxGhosts(8)
	, m_recordInterval(0.02f)
	, m_recordTimer(0.0f)
	, m_alphaDecay(0.78f)
	, m_strengthDecay(0.85f)
	, m_ghostShader(ghostShader)
	, m_lastPushed(cf_v2(0.0f, 0.0f))
	, m_hasLast(false)
{
}

void TrailGhostEffect::trigger(int flashes, float totalDuration, float maxIntensity)
{
	// Map provided parameters to internal configuration.
	// flashes -> desired ghost count
	// totalDuration -> how long the effect persists
	// maxIntensity -> base alpha
	m_maxGhosts = std::max(1, flashes);
	m_totalDuration = std::max(0.1f, totalDuration);
	m_baseAlpha = cf_clamp(maxIntensity, 0.05f, 1.0f);

	m_positions.clear();
	m_elapsed = 0.0f;
	m_recordTimer = 0.0f;
	m_active = true;
	m_hasLast = false;
}

void TrailGhostEffect::update(float dt)
{
	if (!m_active) return;
	m_elapsed += dt;
	if (m_elapsed >= m_totalDuration) {
		m_active = false;
		m_positions.clear();
		invokeOnCompleteOnce();
		return;
	}
	// Position sampling is performed via updateSubjectPosition, called externally.
	// We only advance internal timers here.
	m_recordTimer -= dt;
}

void TrailGhostEffect::beginDraw()
{
	// No-op for ghost trail; ghosts are drawn separately by the subject.
}

void TrailGhostEffect::endDraw()
{
	// No-op.
}

bool TrailGhostEffect::isActive() const
{
	return m_active;
}

void TrailGhostEffect::updateSubjectPosition(v2 position)
{
	if (!m_active) return;

	// Rate-limit sampling for smoother ghost spacing.
	if (m_recordTimer > 0.0f) return;
	m_recordTimer = m_recordInterval;

	if (!m_hasLast || cf_len(cf_v2(position.x - m_lastPushed.x, position.y - m_lastPushed.y)) > 0.1f) {
		// Push newest to back, oldest at front. Cap size.
		m_positions.push_back(position);
		m_lastPushed = position;
		m_hasLast = true;
		while ((int)m_positions.size() > m_maxGhosts) {
			m_positions.pop_front();
		}
	}
}

int TrailGhostEffect::getGhostCount() const
{
	return (int)m_positions.size();
}

v2 TrailGhostEffect::getGhostPosition(int index) const
{
	if (index < 0 || index >= (int)m_positions.size()) return cf_v2(0.0f, 0.0f);
	return m_positions[index];
}

float TrailGhostEffect::getGhostAlpha(int index) const
{
	// Oldest at index 0 -> lowest alpha; newest at back -> higher alpha.
	int n = (int)m_positions.size();
	if (n == 0) return 0.0f;
	// Map index 0..n-1 to depth 0..n-1 from oldest to newest
	int depthFromNewest = (n - 1) - index;
	float falloff = 1.0f;
	for (int i = 0; i < depthFromNewest + 1; ++i) falloff *= m_alphaDecay;
	return cf_clamp(m_baseAlpha * falloff, 0.0f, 1.0f);
}

CF_Shader TrailGhostEffect::getGhostShader() const
{
	return m_ghostShader;
}

float TrailGhostEffect::getGhostStrength(int index) const
{
	int n = (int)m_positions.size();
	if (n == 0) return 0.0f;
	int depthFromNewest = (n - 1) - index;
	float strength = 1.0f;
	for (int i = 0; i < depthFromNewest + 1; ++i) strength *= m_strengthDecay;
	return cf_clamp(strength, 0.0f, 1.0f);
}


