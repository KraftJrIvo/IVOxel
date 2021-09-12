#pragma once

#include <opencv2/opencv.hpp>

#include <GameState.h>
#include <VoxelMapRayTracer.h>

#include "AbstractRenderer.h"

class CPURenderer : public AbstractRenderer
{
public:
	CPURenderer(Window& w, GameState& gs) : 
		AbstractRenderer(w, gs, true),
		_raytracer(gs.getMap(), true)
	{ }

	void startRender() override;
	void stop() override;

private:
	VoxelMapRayTracer _raytracer;

	void _drawImage(cv::Mat img);
	bool _runOnce();

	std::vector<uint8_t> _renderPixel(glm::vec2 xy) const;
};