#pragma once

#include <vector>

#include "VoxelMap.h"

class VoxelMapRayTracer
{
public:
	VoxelMapRayTracer(VoxelMap& map, uint32_t chunkLoadRadius, float epsilon, bool alignToFourBytes = true);
	void setMapData(const std::vector<uint8_t>& data);
	void setLightData(glm::vec3 ambientLightDir, glm::vec4 ambientLightColor, const std::vector<uint8_t>& lights);
	glm::vec3 raytraceMap(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& normal, glm::vec3& color, bool light = false) const;

private:
	uint32_t _chunkLoadRadius;
	uint32_t _chunkLoadDiameter;
	float _epsilon;
	VoxelMapFormat _format;
	bool _alignToFourBytes;
	uint32_t _nLights;
	VoxelTypeStorer& _vts;
	std::vector<uint8_t> _mapData;
	std::vector<uint8_t> _lightData;
	glm::vec3 _ambientLightDir;
	glm::vec4 _ambientLightColor;

	bool _raytraceChunk(const VoxelChunkState& chunkH, glm::vec3 rayStart, glm::vec3 rayDir, glm::ivec3 curChunkPos, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false) const;
	bool _raytraceVoxel(glm::uint voxOff, const VoxelNeighbours& neighs, glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 absPos, float voxRatio, glm::vec3& hitPoint, glm::vec3& normal, glm::vec3& color, bool light = false) const;

	bool _checkBounds(glm::vec3 absPos) const;
	glm::vec3 _getCurEntryPoint(glm::vec3 absPos, float side, glm::vec3 lastRes) const;
	glm::vec3 _marchAndGetNextDir(glm::vec3 dir, float side, glm::ivec2 minmax, glm::uvec3* parals, bool& finish, glm::vec3& absPos, glm::vec3& lastRes, glm::vec3& absCoord) const;
};