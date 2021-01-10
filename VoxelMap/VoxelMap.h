#pragma once

#include <map>

#include <Eigen/Core>

#include "VoxelChunkGenerator.h"
#include "VoxelChunkStorer.h"
#include "VoxelChunk.h"
#include "Light.h"

class VoxelMap
{
public:
	VoxelMap() = default;
	VoxelMap(const VoxelMapFormat& format, const VoxelChunkGenerator& generator, uint32_t loadRadius = 7);

	VoxelMapFormat getFormat() const;
	VoxelChunk& getChunk(const std::vector<int32_t>& pos);
	std::vector<uint8_t> getChunksDataAt(const std::vector<int32_t>& absPos, uint8_t radius, bool alignToFourBytes = true);
	std::vector<uint8_t> getChunkParals(const std::vector<int32_t>& pos);
	
	uint32_t addLight(const Light& l);
	void moveLight(uint32_t lightID, const std::vector<float>& absPos);
	void removeLight(uint32_t lightID);
	std::vector<Light> getLightsByChunk(const std::vector<int32_t>& pos, uint32_t radius = 0) const;

	bool checkLoadNeeded(const std::vector<int32_t>& pos);

protected:
	uint32_t _loadRadius;
	uint32_t _loadDiameter;
	VoxelChunk _emptyChunk;

	VoxelMapFormat _format;
	const VoxelChunkGenerator& _generator;
	VoxelChunkStorer _storer;

	std::vector<VoxelChunk> _chunks;

	uint32_t _lastLightId = 0;
	std::map<uint32_t, Light> _lights;

	std::vector<int32_t> _curAbsPos;

	uint32_t _getIdx(const std::vector<int32_t>& pos) const;
	bool _checkParal(const std::vector<int16_t>& from, const std::vector<int16_t>& to);
	bool _checkParalDist(const std::vector<int16_t>& from, const std::vector<int16_t>& to, const std::vector<int8_t>& dir);
};