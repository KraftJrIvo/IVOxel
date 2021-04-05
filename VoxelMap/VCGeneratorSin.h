#pragma once

#include "VoxelChunkGenerator.h"

class VCGeneratorSin : public VoxelChunkGenerator
{
public:
	VoxelChunk generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const override;
	std::vector<Light> generateLights(const std::vector<int32_t>& pos, float radius, float time = 0.0f) const override;

	void setGroundType(const std::pair<std::shared_ptr<VoxelShape>, std::shared_ptr<VoxelMaterial>>& gt);

private:
	std::pair<std::shared_ptr<VoxelShape>, std::shared_ptr<VoxelMaterial>> _groundType;
};