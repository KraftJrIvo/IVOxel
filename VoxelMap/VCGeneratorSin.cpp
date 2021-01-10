#include "VCGeneratorSin.h"

VoxelChunk VCGenerator::generate(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const
{
    std::vector<Voxel> voxels(pow(side, 3));

    float offset = 1.0f / side;

    for (uint32_t x = 0; x < side; ++x)
        for (uint32_t y = 0; y < side; ++y)
            for (uint32_t z = 0; z < side; ++z)
            {
                float val = 0.5f + (sin(pos[0] + x) + sin(pos[2] + z)) / 4.0f;
                bool ground = y < val;
                Voxel v = Voxel(0, ground ? VoxelType::CUBE : VoxelType::AIR, VoxelOrientation::DEFAULT, { 100, 100, 255, 255 });
                voxels.push_back(v);
            }

    return VoxelChunk(voxels, format.chunkFormat, format.voxelFormat);
}
