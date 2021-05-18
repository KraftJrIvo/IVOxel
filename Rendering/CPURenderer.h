#pragma once

#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include <GameState.h>
#include <VoxelMapRayTracer.h>

#include "IVoxelRenderer.h"

class CPURenderer : public IVoxelRenderer
{
public:
	CPURenderer(Window& w, GameState& gs, uint32_t chunkLoadRadius = 1, float epsilon = 0.00001, bool alignToFourBytes = true) :
		_window(w),
		_gs(gs),
		_raytracer(*gs.getMap(), chunkLoadRadius, epsilon, alignToFourBytes),
		_chunkLoadRadius(chunkLoadRadius),
		_alignToFourBytes(alignToFourBytes),
		_firstRender(true),
		_time(0)
	{ }

	void startRender() override;
	void stop() override;

private:
	Window& _window;
	GameState& _gs;

	VoxelMapRayTracer _raytracer;

	long _time;

	uint32_t _chunkLoadRadius;
	bool _alignToFourBytes;
	bool _firstRender;

	void _drawImage(cv::Mat img);
	bool _runOnce();

	std::vector<uint8_t> _renderPixel(const VoxelMap& map, const Camera& cam, glm::vec2 xy) const;
};