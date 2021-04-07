#include "VCGeneratorSin.h"

VoxelChunk VCGeneratorSin::generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const
{
    std::vector<Voxel> voxels(pow(side, 3));

    float offset = 1.0f / side;

    for (uint32_t x = 0; x < side; ++x)
        for (uint32_t y = 0; y < side; ++y)
            for (uint32_t z = 0; z < side; ++z)
            {
                float val = 0.5f + (sin(pos[0] + x) + sin(pos[2] + z)) / 4.0f;
                bool ground = y < val;
                Voxel v = Voxel(ground ? _groundType.first : nullptr, ground ? _groundType.second : nullptr, {{0,0,0}, false}, { 100, 100, 255, 255 });
                voxels[side * side * z + side * y + x] = v;
            }

    return VoxelChunk(voxels, format.chunkFormat, format.voxelFormat);
}

std::vector<Light> VCGeneratorSin::generateLights(const std::vector<int32_t>& pos, float radius, float time) const
{
    int LIGHTS_PER_N_CHUNKS = 3;
    float diam = radius * 2.0f;

    std::vector<Light> lights;

    for (int i = 0; i < diam; ++i)
    {
        for (int j = 0; j < diam; ++j)
        {
            int xCoord = i + (int)floor(pos[0]);
            int zCoord = j + (int)floor(pos[2]);
            if (xCoord % LIGHTS_PER_N_CHUNKS == 0 && zCoord % LIGHTS_PER_N_CHUNKS == 0)
                lights.push_back(Light({ xCoord + 0.5f, 3.0f, zCoord + 0.5f }, {255, 255, 255, 255}));
        }
    }
    return lights;
}

void VCGeneratorSin::setGroundType(const std::pair<std::shared_ptr<VoxelShape>, std::shared_ptr<VoxelMaterial>>& gt)
{
    _groundType = gt;
}
