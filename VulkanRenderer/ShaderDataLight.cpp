#include "ShaderDataLight.h"

#include "VulkanDescriptorPool.h"
#include "GameState.h"

void ShaderDataLight::update(VulkanDescriptorPool& pool, uint32_t frameID, uint32_t dataID, GameState& game)
{
	auto lsbc = game.getMap().getLightsByChunks();
	uint32_t curLight = 0;
	const size_t colorSz = 3 * sizeof(float);
	const size_t posSz = DIMENSIONS * sizeof(float);
	static float t = 0;
	for (auto& ls : lsbc)
	{
		for (auto& l : ls)
		{
			std::vector<float> color = { l.rgba[R] / 255.0f, l.rgba[G] / 255.0f, l.rgba[B] / 255.0f };

			auto pos = l.position;

			if (&l == &*ls.begin())
			{
				pos[0] = pos[0] + cos(t / 10.);
				pos[1] = pos[1] + sin(t / 5.);
				pos[2] = pos[2] + sin(t / 10.);
			}
			else
			{
				pos[0] = pos[0] + cos(-t);
				pos[2] = pos[2] + sin(-t);
			}

			state.coords[curLight].x = pos[0];
			state.coords[curLight].y = pos[1];
			state.coords[curLight].z = pos[2];
			state.colors[curLight].x = color[0];
			state.colors[curLight].y = color[1];
			state.colors[curLight].z = color[2];

			curLight++;
		}
	}
	state.nLights = curLight;

	pool.setData(frameID, dataID, &state);
	t += 0.01f;
}
