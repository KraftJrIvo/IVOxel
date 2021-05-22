#pragma once

#include <map>

#include "VoxelChunkGenerator.h"
#include "VoxelChunkStorer.h"
#include "VoxelChunk.h"
#include "Light.h"

class VoxelMap
{
public:
	VoxelMap() = default;
	VoxelMap(const VoxelMapFormat& format, VoxelChunkGenerator& generator, uint8_t chunkSide = 16, uint8_t loadRadius = 7, uint8_t maxLights = 10);

	VoxelMapFormat getFormat() const;
	VoxelTypeStorer& getVoxelTypeStorer();
	VoxelChunk& getChunk(const std::vector<int32_t>& relPos);
	std::vector<uint8_t> getChunksDataAt(const std::vector<int32_t>& absPos, uint8_t radius, bool alignToFourBytes = true);
	std::vector<uint8_t> getChunkParals(const std::vector<int32_t>& pos);
	std::vector<uint8_t> getLightDataAt(const std::vector<int32_t>& absPos, uint8_t radius, float time = 0.0f) const;

	uint8_t getLoadRadius();
	uint8_t getChunkSide();
	uint8_t getMaxLights();
	uint32_t getMapDataSize(bool alignToFourBytes = true);
	
	uint32_t addLight(const Light& l);
	void moveLight(uint32_t lightID, const std::vector<float>& absPos);
	void removeLight(uint32_t lightID);

	bool checkAndLoad(const std::vector<int32_t>& pos);

	void setAbsPos(const std::vector<int32_t>& absPos);

protected:
	uint8_t _loadRadius;
	uint32_t _loadDiameter;
	uint8_t _chunkSide;
	uint8_t _maxLights;
	VoxelChunk _emptyChunk;

	VoxelTypeStorer _voxTypeStorer;

	VoxelMapFormat _format;
	VoxelChunkGenerator& _generator;
	VoxelChunkStorer _storer;

	std::vector<std::shared_ptr<VoxelChunk>> _chunks;

	uint32_t _lastLightId = 0;
	std::map<uint32_t, Light> _lights;

	std::vector<int32_t> _curAbsPos;

	uint32_t _getIdx(const std::vector<int32_t>& pos) const;
	std::vector<int32_t> _getAbsPos(const std::vector<int32_t>& pos) const;
	void _loadChunks();
	void _loadChunk(const std::vector<int32_t>& pos, uint32_t id);
	
	bool _checkParal(std::vector<int16_t> from, std::vector<int16_t> to);
	bool _checkParalDist(std::vector<int16_t> from, std::vector<int16_t> to, const std::vector<int8_t>& dir);
};