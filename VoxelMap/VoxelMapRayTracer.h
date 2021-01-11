#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "VoxelMapFormat.h"

class VoxelMapRayTracer
{
public:
	VoxelMapRayTracer(const VoxelMapFormat& format, uint32_t chunkLoadRadius, bool alignToFourBytes = true);
	void setMapData(const std::vector<uint8_t>& data);
	void raytraceMap(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);

private:
	const uint32_t _chunkLoadRadius;
	const uint32_t _chunkLoadDiameter;
	const VoxelMapFormat& _format;
	const bool _alignToFourBytes;
	std::vector<uint8_t> _mapData;

	bool _rayTraceChunk(glm::vec3 rayStart, glm::vec3 rayDir, glm::ivec3 curChunkPos, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);
	bool _rayTraceVoxel(int voxType, glm::vec3 voxColor, glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 absPos, float voxRatio, glm::vec3& absCoord, glm::vec3& normal, glm::vec3& color);
	glm::vec3 _getVoxelData(int chunkOffset, glm::vec3 pos, uint8_t& voxPower, int& voxType);
	glm::vec3 _getCurEntryPoint(glm::vec3 absPos, float side, glm::vec3 lastRes);
	glm::vec3 _marchAndGetNextDir(glm::vec3 dir, float side, glm::ivec2 minmax, bool& finish, glm::vec3& absPos, glm::vec3& lastRes);
};