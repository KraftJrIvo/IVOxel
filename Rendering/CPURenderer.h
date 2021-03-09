#pragma once

#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include <GameState.h>
#include <VoxelMapRayTracer.h>

#include "IVoxelRenderer.h"

class CPURenderer : public IVoxelRenderer
{
public:
	CPURenderer(GameState& gs, uint32_t chunkLoadRadius = 1, float epsilon = 0.001, bool alignToFourBytes = true) : 
		_gs(gs),
		_raytracer(*gs.getMap(), chunkLoadRadius, epsilon, alignToFourBytes),
		_chunkLoadRadius(chunkLoadRadius),
		_alignToFourBytes(alignToFourBytes)
	{ }

	void startRender() override;
	void stop() override;

private:
	GameState& _gs;
	VoxelMapRayTracer _raytracer;

	uint32_t _chunkLoadRadius;
	bool _alignToFourBytes;

	void _drawImage(cv::Mat img);
	bool _runOnce();

	std::vector<uint8_t> _renderPixel(const VoxelMap& map, const Camera& cam, glm::vec2 xy) const;
};