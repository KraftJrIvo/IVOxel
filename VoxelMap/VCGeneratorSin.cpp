#include "VCGeneratorSin.h"

#include <glm/gtc/matrix_transform.hpp>

std::shared_ptr<VoxelChunk> VCGeneratorSin::generateChunk(const VoxelMapFormat& format, uint32_t side, const std::vector<int32_t>& pos) const
{
    std::vector<Voxel> voxels(pow(side, 3));

    float offset = 1.0f / side;

    for (uint32_t x = 0; x < side; ++x)
        for (uint32_t y = 0; y < side; ++y)
            for (uint32_t z = 0; z < side; ++z)
            {
                float val = 0.5f + (sin(pos[0] + x * offset) + sin(pos[2] + z * offset)) / 4.0f;
                bool ground = (y * offset + pos[1]) < val;
                Voxel v = Voxel(log(side)/log(2), ground ? _groundType.first : nullptr, ground ? _groundType.second : nullptr, {{0,0,0}, false}, { 150, 150, 150, 255 });
                voxels[side * side * z + side * y + x] = v;
            }

    return std::make_shared<VoxelChunk>(voxels, format.chunkFormat, format.voxelFormat);
}

std::vector<Light> VCGeneratorSin::generateLights(const std::vector<int32_t>& pos, float radius, float time) const
{
    int LIGHTS_PER_N_CHUNKS = 3;
    int diam = radius * 2 + 1;

    std::vector<Light> lights;

    glm::vec3 dir = { 1.0f, 0.2f, 0.0f };
    glm::mat4 rotmat(1.0);
    rotmat = glm::rotate(rotmat, glm::radians(time * 3), glm::vec3(0.0f, 1.0f, 0.0f));
    auto sun = glm::mat3(rotmat) * dir;

    lights.push_back(Light(LightType::AMBIENT, { 10, 10, 10, 255 }));
    lights.push_back(Light(LightType::GLOBAL, { 180, 180, 180, 255 }, { sun.x, sun.y, sun.z }));
    
    for (int i = 1; i < diam; ++i)
    {
        for (int j = 1; j < diam; ++j)
        {
            if (int(pos[0] + i - radius) % 2 == 0 && int(pos[2] + j - radius) % 2 == 0)
            {
                float xCoord = 0.51 + i - radius;
                float zCoord = 0.51 + j - radius;
                std::vector<float> curpos = { xCoord, 0.9f - pos[1] + sin(time * 3.0f + (pos[0] + i)) / 3.0f + cos(time * 3.0f + (pos[2] + j)) / 3.0f, zCoord };
                std::vector<unsigned char> curcol = { (unsigned char)(85 + (pos[0] + i) * 300), 0, (unsigned char)(85 + (pos[2] + j) * 300), 255 };
                lights.push_back(Light(LightType::LOCAL, curcol, curpos));
            }
        }
    }

    return lights;
}

void VCGeneratorSin::setGroundType(const std::pair<std::shared_ptr<VoxelShape>, std::shared_ptr<VoxelMaterial>>& gt)
{
    _groundType = gt;
}
