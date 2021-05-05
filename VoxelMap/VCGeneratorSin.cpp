#include "VCGeneratorSin.h"

std::shared_ptr<VoxelChunk> VCGeneratorSin::generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const
{
    std::vector<Voxel> voxels(pow(side, 3));

    float offset = 1.0f / side;

    for (uint32_t x = 0; x < side; ++x)
        for (uint32_t y = 0; y < side; ++y)
            for (uint32_t z = 0; z < side; ++z)
            {
                float val = 0.5f + (sin(pos[0] + x * offset) + sin(pos[2] + z * offset)) / 4.0f;
                bool ground = (y * offset + pos[1]) < 0.25;// val;
                Voxel v = Voxel(log(side)/log(2), ground ? _groundType.first : nullptr, ground ? _groundType.second : nullptr, {{0,0,0}, false}, { 125 + pos[0] * 80, 125 + pos[1] * 80, 125 + pos[2] * 80, 255 });
                voxels[side * side * z + side * y + x] = v;
            }

    return std::make_shared<VoxelChunk>(voxels, format.chunkFormat, format.voxelFormat);
}

std::vector<Light> VCGeneratorSin::generateLights(const std::vector<int32_t>& pos, float radius, float time) const
{
    int LIGHTS_PER_N_CHUNKS = 3;
    float diam = radius * 2.0f + 1.0f;

    std::vector<Light> lights;

    for (int i = 0; i < diam; ++i)
    {
        for (int j = 0; j < diam; ++j)
        {
            int xCoord = i + (int)floor(pos[0] - diam/2.0f);
            int zCoord = j + (int)floor(pos[2] - diam/2.0f);
            if (xCoord % LIGHTS_PER_N_CHUNKS == 0 && zCoord % LIGHTS_PER_N_CHUNKS == 0)
                lights.push_back(Light({ xCoord + 0.5f, 1.8f, zCoord + 0.5f }, {255, 255, 255, 255}));
        }
    }
    return lights;
}

void VCGeneratorSin::setGroundType(const std::pair<std::shared_ptr<VoxelShape>, std::shared_ptr<VoxelMaterial>>& gt)
{
    _groundType = gt;
}
