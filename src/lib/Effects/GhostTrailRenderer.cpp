#include "GhostTrailRenderer.h"
#include "../Character/AnimatedDataCharacter.h"
#include "IGhostTrailEffect.h"
#include <cute_draw.h>

using namespace Cute;

void GhostTrailRenderer::renderGhostsForCharacter(AnimatedDataCharacter& character)
{
	IGhostTrailEffect* ghost = character.getActiveGhostTrailEffect();
	if (!ghost) return;

	int count = ghost->getGhostCount();
	if (count <= 0) return;

	CF_Shader ghostShader = ghost->getGhostShader();
	for (int i = 0; i < count; ++i)
	{
		v2 gp = ghost->getGhostPosition(i);
		float a = ghost->getGhostAlpha(i);
		float strength = ghost->getGhostStrength(i);

		bool pushedShader = false;
		if (ghostShader.id != 0)
		{
			cf_draw_push_shader(ghostShader);
			cf_draw_set_uniform("strength", &strength, CF_UNIFORM_TYPE_FLOAT, 1);
			pushedShader = true;
		}
		// For premultiplied pipeline, scale rgb and a equally.
		CF_Color ghostColor = make_color(a, a, a, a);
		cf_draw_push_color(ghostColor);

		// Use character's frame rendering helper at a specific world position.
		cf_draw_push();
		character.renderCurrentFrameAt(gp);
		cf_draw_pop();

		cf_draw_pop_color();
		if (pushedShader) cf_draw_pop_shader();
	}
}


