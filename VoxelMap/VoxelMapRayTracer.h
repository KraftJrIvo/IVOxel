#pragma once

#include <vector>

#include "VoxelType.h"
#include "VoxelMapFormat.h"

class VoxelMapRayTracer
{
public:
	VoxelMapRayTracer(const VoxelMapFormat& format, VoxelTypeStorer& vts, uint32_t chunkLoadRadius, float epsilon, bool alignToFourBytes = true);
	void setMapData(const std::vector<uint8_t>& data);
	void raytraceMap(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);

private:
	const uint32_t _chunkLoadRadius;
	const uint32_t _chunkLoadDiameter;
	const float _epsilon;
	const VoxelMapFormat& _format;
	const bool _alignToFourBytes;
	VoxelTypeStorer& _vts;
	std::vector<uint8_t> _mapData;

	bool _rayTraceChunk(const VoxelChunkHeader& chunkH, glm::vec3 rayStart, glm::vec3 rayDir, glm::ivec3 curChunkPos, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);
	bool _rayTraceVoxel(const Voxel& vox, glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 absPos, float voxRatio, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);
	glm::vec3 _getCurEntryPoint(glm::vec3 absPos, float side, glm::vec3 lastRes);
	glm::vec3 _marchAndGetNextDir(glm::vec3 dir, float side, glm::ivec2 minmax, glm::vec4 parals, bool& finish, glm::vec3& absPos, glm::vec3& lastRes, glm::vec3& absCoord);
};