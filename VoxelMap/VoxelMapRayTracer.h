#pragma once

#include <vector>

#include "VoxelType.h"
#include "VoxelMapFormat.h"

class VoxelMapRayTracer
{
public:
	VoxelMapRayTracer(const VoxelMapFormat& format, VoxelTypeStorer& vts, uint32_t chunkLoadRadius, float epsilon, bool alignToFourBytes = true);
	void setMapData(const std::vector<uint8_t>& data);
	void setLightData(glm::vec3 ambientLightDir, glm::vec4 ambientLightColor, const std::vector<uint8_t>& lights);
	glm::vec3 raytraceMap(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& normal, glm::vec3& color, bool light = false);

private:
	const uint32_t _chunkLoadRadius;
	const uint32_t _chunkLoadDiameter;
	const float _epsilon;
	const VoxelMapFormat& _format;
	const bool _alignToFourBytes;
	uint32_t _nLights;
	VoxelTypeStorer& _vts;
	std::vector<uint8_t> _mapData;
	std::vector<uint8_t> _lightData;
	glm::vec3 _ambientLightDir;
	glm::vec4 _ambientLightColor;

	bool _raytraceChunk(const VoxelChunkState& chunkH, glm::vec3 rayStart, glm::vec3 rayDir, glm::ivec3 curChunkPos, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false);
	bool _raytraceVoxel(glm::uint voxOff, const VoxelNeighbours& neighs, glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 absPos, float voxRatio, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false);

	glm::vec3 _getCurEntryPoint(glm::vec3 absPos, float side, glm::vec3 lastRes);
	glm::vec3 _marchAndGetNextDir(glm::vec3 dir, float side, glm::ivec2 minmax, glm::uvec3* parals, bool& finish, glm::vec3& absPos, glm::vec3& lastRes, glm::vec3& absCoord);
};