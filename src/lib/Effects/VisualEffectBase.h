#ifndef VISUAL_EFFECT_BASE_H
#define VISUAL_EFFECT_BASE_H

#include "IVisualEffect.h"
#include <functional>

class VisualEffectBase : public IVisualEffect
{
public:
	void setOnComplete(std::function<void()> callback) override { m_onComplete = std::move(callback); }

protected:
	void invokeOnCompleteOnce() { if (m_onComplete) { auto cb = m_onComplete; m_onComplete = nullptr; cb(); } }

private:
	std::function<void()> m_onComplete;
};

#endif // VISUAL_EFFECT_BASE_H


