#include "DissolveEffect.h"

DissolveEffect::DissolveEffect(CF_Shader shader)
	: m_active(false)
	, m_elapsed(0.0f)
	, m_totalDuration(1.0f)
	, m_edgeWidth(0.06f)
	, m_shader(shader)
	, m_shaderPushed(false)
{
}

void DissolveEffect::trigger(int /*flashes*/, float totalDuration, float maxIntensity)
{
	m_totalDuration = totalDuration > 0.05f ? totalDuration : 0.05f;
	// Use maxIntensity as edge width clamp.
	m_edgeWidth = cf_clamp(maxIntensity, 0.0f, 0.25f);
	m_elapsed = 0.0f;
	m_active = true;
}

void DissolveEffect::update(float dt)
{
	if (!m_active) return;
	m_elapsed += dt;
	if (m_elapsed >= m_totalDuration) {
		m_active = false;
		m_elapsed = m_totalDuration;
		invokeOnCompleteOnce();
	}
}

bool DissolveEffect::isActive() const
{
	return m_active;
}

void DissolveEffect::beginDraw()
{
	m_shaderPushed = false;
	if (!m_active) return;
	if (m_shader.id == 0) return;

	// threshold ramps 0..1 over duration
	float t = (m_totalDuration > 0.0f) ? cf_clamp(m_elapsed / m_totalDuration, 0.0f, 1.0f) : 1.0f;
	float threshold = t;
	float edgeWidth = m_edgeWidth;
	float time = m_elapsed;
	CF_Color edgeColor = cf_make_color_rgb(255, 120, 20); // warm edge
	float edge[3] = { edgeColor.r, edgeColor.g, edgeColor.b };

	cf_draw_push_shader(m_shader);
	cf_draw_set_uniform("threshold", &threshold, CF_UNIFORM_TYPE_FLOAT, 1);
	cf_draw_set_uniform("edgeWidth", &edgeWidth, CF_UNIFORM_TYPE_FLOAT, 1);
	cf_draw_set_uniform("time", &time, CF_UNIFORM_TYPE_FLOAT, 1);
	cf_draw_set_uniform("edgeColor", edge, CF_UNIFORM_TYPE_FLOAT, 3);
	m_shaderPushed = true;
}

void DissolveEffect::endDraw()
{
	if (m_shaderPushed) {
		cf_draw_pop_shader();
		m_shaderPushed = false;
	}
}

void DissolveEffect::setOnComplete(std::function<void()> callback)
{
	m_onComplete = std::move(callback);
}


