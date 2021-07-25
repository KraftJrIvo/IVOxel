#pragma once

#include <GameState.h>
#include <VoxelMapRayTracer.h>

class AbstractRenderer
{
public:
	AbstractRenderer(Window& w, GameState& gs, bool alignToFourBytes) :
		_window(w), 
		_gs(gs), 
		_alignToFourBytes(alignToFourBytes)
	{ }

	virtual void startRender() = 0;
	virtual void stop() = 0;

protected:
	Window& _window;
	GameState& _gs;

	bool _firstRender = true;
	bool _alignToFourBytes;
};