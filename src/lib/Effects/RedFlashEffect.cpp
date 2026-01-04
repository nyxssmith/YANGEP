#include "RedFlashEffect.h"
#include <cmath>

RedFlashEffect::RedFlashEffect(CF_Shader shader)
	: m_active(false)
	, m_elapsed(0.0f)
	, m_flashes(3)
	, m_totalDuration(2.0f)
	, m_maxIntensity(0.85f)
	, m_shader(shader)
	, m_shaderPushed(false)
{
}

void RedFlashEffect::trigger(int flashes, float totalDuration, float maxIntensity)
{
	m_flashes = flashes;
	m_totalDuration = totalDuration;
	m_maxIntensity = maxIntensity;
	m_elapsed = 0.0f;
	m_active = true;
}

void RedFlashEffect::update(float dt)
{
	if (!m_active) return;
	m_elapsed += dt;
	if (m_elapsed >= m_totalDuration) {
		m_active = false;
		m_elapsed = 0.0f;
	}
}

bool RedFlashEffect::isActive() const
{
	return m_active;
}

float RedFlashEffect::computeIntensity() const
{
	if (!m_active || m_totalDuration <= 0.0f || m_flashes <= 0) return 0.0f;
	float segment = m_totalDuration / (float)m_flashes;
	if (segment <= 0.0f) return 0.0f;
	float t = m_elapsed;
	if (t >= m_totalDuration) return 0.0f;
	float local = fmodf(t, segment);
	float phase = local / segment;
	float pulse = 0.5f * (1.0f - cosf(phase * 2.0f * 3.14159265f));
	return m_maxIntensity * pulse;
}

void RedFlashEffect::beginDraw()
{
	m_shaderPushed = false;
	if (!m_active) return;
	if (m_shader.id == 0) return;
	float intensity = computeIntensity();
	if (intensity <= 0.0f) return;
	cf_draw_push_shader(m_shader);
	cf_draw_set_uniform("intensity", &intensity, CF_UNIFORM_TYPE_FLOAT, 1);
	m_shaderPushed = true;
}

void RedFlashEffect::endDraw()
{
	if (m_shaderPushed) {
		cf_draw_pop_shader();
		m_shaderPushed = false;
	}
}


