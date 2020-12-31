#pragma once

class VoxelChunkGenerator
{
public:
	VoxelChunkGenerator() = default;
	VoxelChunkGenerator(unsigned long long seed);
	VoxelChunk generate(const std::vector<int32_t>& pos);
private:
	unsigned long long _seed;
};