#pragma once

#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include <GameState.h>
#include <VoxelMapRayTracer.h>

#include "IVoxelRenderer.h"

class CPURenderer : public IVoxelRenderer
{
public:
	CPURenderer(Window& w, GameState& gs, bool alignToFourBytes = true) :
		_window(w),
		_gs(gs),
		_raytracer(gs.getMap(), alignToFourBytes),
		_alignToFourBytes(alignToFourBytes),
		_firstRender(true)
	{ }

	void startRender() override;
	void stop() override;

private:
	Window& _window;
	GameState& _gs;

	VoxelMapRayTracer _raytracer;

	bool _alignToFourBytes;
	bool _firstRender;

	void _drawImage(cv::Mat img);
	bool _runOnce();

	std::vector<uint8_t> _renderPixel(glm::vec2 xy) const;
};