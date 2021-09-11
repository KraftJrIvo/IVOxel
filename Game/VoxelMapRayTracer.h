#pragma once

#include <vector>

#include "GameDataContainer.h"

#include <VoxelMap.h>

class VoxelMapRayTracer : public GameDataContainer
{
public:
	VoxelMapRayTracer(VoxelMap& map, bool alignToFourBytes = true);
	void setData(uint32_t dataID, void* ptr, uint32_t frameID = 0) override;
	bool isGPU() override { return false; }
	glm::vec3 raytracePixel(glm::vec2 xy, glm::vec3& normal, glm::vec3& color) const;
	glm::vec3 raytraceMap(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& normal, glm::vec3& color, bool light = false) const;

private:
	VoxelMapFormat _format;
	bool _alignToFourBytes;
	VoxelTypeStorer& _vts;
	
	const uint8_t* _camData;
	const uint8_t* _mapData;
	const uint8_t* _lightData;
	const uint8_t* _constData;

	bool _raytraceChunk(const VoxelChunkState& chunkH, glm::vec3 rayStart, glm::vec3 rayDir, glm::ivec3 curChunkPos, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false) const;
	bool _raytraceVoxel(glm::uint voxOff, const VoxelNeighbours& neighs, glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 absPos, float voxRatio, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false) const;

	void _drawLights(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& absPos, glm::vec3& color) const;

	bool _checkMapBounds(glm::vec3 absPos) const;
	bool _checkChunkBounds(glm::vec3 pos, uint32_t steps) const;
	glm::vec3 _getCurEntryPoint(glm::vec3 absPos, float side, glm::vec3 lastRes) const;
	glm::vec3 _marchAndGetNextDir(glm::vec3 dir, float side, glm::ivec2 minmax, glm::uvec3* parals, bool& finish, glm::vec3& absPos, glm::vec3& lastRes, glm::vec3& absCoord) const;
};