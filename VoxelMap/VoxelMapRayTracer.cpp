#include "VoxelMapRayTracer.h"
#include <iostream>

using namespace glm;

#define deb false

VoxelMapRayTracer::VoxelMapRayTracer(VoxelMap& map, uint32_t chunkLoadRadius, float epsilon, bool alignToFourBytes) :
    _vts(map.getVoxelTypeStorer()),
    _format(map.getFormat()),
    _chunkLoadRadius(chunkLoadRadius),
    _chunkLoadDiameter(chunkLoadRadius * 2 + 1),
    _epsilon(epsilon),
    _alignToFourBytes(alignToFourBytes)
{ }

void VoxelMapRayTracer::setMapData(const std::vector<uint8_t>& data)
{
    _mapData = data;
}

void VoxelMapRayTracer::setLightData(glm::vec3 ambientLightDir, glm::vec4 ambientLightColor, const std::vector<uint8_t>& data)
{
    _ambientLightDir = ambientLightDir;
    _ambientLightColor = ambientLightColor;

    _lightData = data;
    _nLights = data.size() / (8 * sizeof(float));
}

vec3 VoxelMapRayTracer::raytraceMap(vec3 rayStart, vec3 rayDir, vec3& normal, vec3& color, bool light) const
{
    vec3 resultColor = vec3(0, 0, 0);
    vec3 lastRes = vec3(0, 0, 0);
    
    ivec3 curChunkPos = ivec3(floor(rayStart));
    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;

    if (deb) std::cout << "start\n";

    while (notFinish && !marchFinish)
    {
        if (deb) std::cout << "chunk " << curChunkPos[0] << " " << curChunkPos[1] << " " << curChunkPos[2] << "\n";
        if (_checkMapBounds({ curChunkPos[0], curChunkPos[1], curChunkPos[2] })) {
            color = { 255, 0, 0 };
            break;
        }

        uint32_t idx = (curChunkPos[0] + _chunkLoadRadius) * _chunkLoadDiameter * _chunkLoadDiameter + (curChunkPos[1] + _chunkLoadRadius) * _chunkLoadDiameter + curChunkPos[2] + _chunkLoadRadius;
        uint32_t off = _format.chunkFormat.getSizeInBytes() * idx;
        auto chunkH = _format.getChunkState(_mapData.data() + off, _alignToFourBytes);

        vec3 prevHitPoint = marchAbsPos;
        notFinish = !chunkH.fullness || !_raytraceChunk(chunkH, _getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, marchAbsPos, normal, color, light);

        if (notFinish)
        {
            marchAbsPos = prevHitPoint;
            vec3 coord;
            chunkH.parals.data();
            _marchAndGetNextDir(rayDir, 1, ivec2(-(int32_t)_chunkLoadRadius, _chunkLoadRadius + 1), chunkH.parals.data(), marchFinish, marchAbsPos, lastRes, coord);
            curChunkPos = ivec3(floor(marchAbsPos));
        }
    }

        if (marchFinish)
        color = { 255, 0, 0 };

    return marchAbsPos;
}

bool VoxelMapRayTracer::_raytraceChunk(const VoxelChunkState& chunkH, vec3 rayStart, vec3 rayDir, ivec3 curChunkPos, vec3& absCoord, vec3& normal, vec3& color, bool light) const
{
    vec3 resultColor = vec3(0, 0, 0);
    float sideSteps = chunkH.side;
    vec3 marchPos = rayStart * sideSteps;
    vec3 lastRes = vec3(0);
    bool marchFinish = false;
    uvec3 curVoxPos = ivec3(floor(marchPos));
    uint stepsToTake = 1;
    bool keepTracing = chunkH.fullness;

    while (keepTracing)
    {
        if (deb) std::cout << "vox " << curVoxPos[0] << " " << curVoxPos[1] << " " << curVoxPos[2] << "\n";
        if (_checkChunkBounds(curVoxPos, sideSteps))
            break;

        glm::uint voxOff = chunkH.voxOffset + (curVoxPos[0] * chunkH.side * chunkH.side + curVoxPos[1] * chunkH.side + curVoxPos[2]) * _format.voxelFormat.getSizeInBytes(_alignToFourBytes);
        auto voxelState = _format.getVoxelState(_mapData.data() + voxOff);
        stepsToTake = uint(sideSteps / pow(2, voxelState.size)); //!!!
        vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));

        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (voxelState.full)
        {
            vec3 entry = _getCurEntryPoint(marchPos, stepsToTake, lastRes);
            if (_raytraceVoxel(voxOff, voxelState.neighs, entry, rayDir, absPos, voxRatio, absCoord, normal, color, light))
                return true;
        }
        vec3 absCoordVox = {0,0,0};
        _marchAndGetNextDir(rayDir, stepsToTake, ivec2(0, sideSteps), voxelState.parals.data(), marchFinish, marchPos, lastRes, absCoordVox);
        absCoord += absCoordVox * voxRatio;
        curVoxPos = ivec3(floor(marchPos));
        keepTracing = !marchFinish;
    }

    return false;
}

bool VoxelMapRayTracer::_raytraceVoxel(glm::uint voxOff, const VoxelNeighbours& neighs, vec3 rayStart, vec3 rayDir, vec3 absPos, float voxRatio, vec3& absCoord, vec3& normal, vec3& color, bool light) const
{
    Voxel voxel = _format.voxelFormat.unformatVoxel(_mapData.data() + voxOff);

    if (voxel.isEmpty())
        return false;
    
    vec3 dir = rayDir;
    dir = normalize(dir);
    
    vec3 hitCoord;
    bool hit = voxel.shape->raytrace(rayStart, dir, neighs, hitCoord, normal);
    hitCoord *= voxRatio;
    normal = normalize(normal);

    if (hit)
    {
        vec3 absRayStart = absPos + hitCoord;
        
        if (!light)
        {
            vec3 voxColor = voxel.color / 255.0f;
            color = { 0,0,0 };

            for (uint i = _nLights; i <= _nLights; ++i)
            {
                float* light = ((float*)_lightData.data() + i * 8 * sizeof(float));

                vec3 lCoord;
                vec4 lColor;
                if (i == _nLights)
                {
                    //vec3 v = absPos + voxRatio * 0.45f + float(_chunkLoadRadius);
                    //lCoord = v + _ambientLightDir * (_chunkLoadRadius - _epsilon) - float(_chunkLoadRadius);
                    //for (int j = 0; j < 3; ++j)
                    //    lCoord[j] = std::clamp(lCoord[j], -float(_chunkLoadRadius) + _epsilon, _chunkLoadRadius + 1.0f - _epsilon);
                    lCoord = absRayStart;
                    lCoord.y = _chunkLoadRadius + 1.0f - _epsilon;
                    lColor = _ambientLightColor;
                }
                else
                {
                    lCoord = { light[3], light[2], light[1] };
                    lColor = { light[4], light[5], light[6], light[7] };
                }

                vec3 dirToLight = lCoord - absRayStart;
                float lightDist = length(dirToLight);
                dirToLight = normalize(dirToLight);

                vec3 lightHitPoint, n, c;
                lightHitPoint = raytraceMap(lCoord, -dirToLight, n, c, true);

                float diff = length(lightHitPoint - absRayStart);
                if (diff < _epsilon * 10)
                    color = voxel.material->shade(color, voxColor, absRayStart, normal, dirToLight, lColor);

                ///
                //vec3 isNeg = { 0.0f, 0.0f, 0.0f };
                //auto vec = vec3(1.0, 2.0, 4.0) * (vec3(1, 1, 1) - isNeg);
                //int paralN = vec.x + vec.y + vec.z;

                ////auto voxelState = _format.getVoxelState(_mapData.data() + voxOff + (1 * 4) * _format.voxelFormat.getSizeInBytes(_alignToFourBytes));
                ////uvec3 paral = voxelState.parals[paralN];

                //ivec3 curChunkPos = ivec3(floor(absRayStart));
                //uint32_t idx = (curChunkPos[0] + _chunkLoadRadius) * _chunkLoadDiameter * _chunkLoadDiameter + ((curChunkPos[1] + 1) + _chunkLoadRadius) * _chunkLoadDiameter + curChunkPos[2] + _chunkLoadRadius;
                //uint32_t off = _format.chunkFormat.getSizeInBytes() * idx;
                //auto chunkH = _format.getChunkState(_mapData.data() + off, _alignToFourBytes);
                //uvec3 paral = chunkH.parals[paralN];
                //
                //color = { paral.x / 4.0f, paral.y / 4.0f, paral.z / 4.0f };
                ///

                //color.y -= nn/4.0f * 0.25f;
                //color.z -= nn/4.0f * 0.25f;
            }
        }
        
        absCoord = absRayStart;
        
        return true;
    }
    return false;
}

bool VoxelMapRayTracer::_checkChunkBounds(glm::vec3 pos, uint32_t steps) const
{
    for (int i = 0; i < 3; ++i)
        if (pos[i] < 0 || pos[i] >= steps)
            return true;
    return false;
}

bool VoxelMapRayTracer::_checkMapBounds(glm::vec3 absPos) const
{
    for (int i = 0; i < 3; ++i)
        if (absPos[i] < -(int32_t)_chunkLoadRadius || absPos[i] >= _chunkLoadRadius + 1.0f)
            return true;
    return false;
}

vec3 VoxelMapRayTracer::_getCurEntryPoint(vec3 absPos, float side, vec3 lastRes) const
{
    vec3 result = vec3(0, 0, 0);

    vec3 curPos = absPos / side;

    for (int i = 0; i < 3; ++i)
    {
        while (curPos[i] < 0) curPos[i] += 1.0;
        result[i] = (lastRes[i] < 0) ? 1.0 - _epsilon : (curPos[i] - floor(curPos[i]));
    }

    return result;
}

vec3 VoxelMapRayTracer::_marchAndGetNextDir(vec3 dir, float side, ivec2 minmax, glm::uvec3* parals, bool& finish, vec3& absPos, vec3& lastRes, glm::vec3& absCoord) const
{
    vec3 isNeg;
    for (int i = 0; i < 3; ++i)
        isNeg[i] = dir[i] < 0 ? 1.0 : 0.0;

    vec3 vecStart = _getCurEntryPoint(absPos, side, lastRes);

    auto vec = vec3(1.0, 2.0, 4.0) * (vec3(1,1,1) - isNeg);
    int paralN = vec.x + vec.y + vec.z;
    vec3 paral = vec3(parals[paralN]);

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        if (side > 1 && paral[i] > 0) paral[i] -= side - 1.0f;
        path[i] = (isNeg[i] != 0) ? (-side * vecStart[i] - (float)paral[i]) : (paral[i] + side * (1.0f - vecStart[i])); //!!!
        path[i] = (path[i] == 0) ? _epsilon : path[i];
    }

    vec3 diffs = dir / path;
    float maxDiff = max(diffs.x, max(diffs.y, diffs.z));

    vec3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = (1.0 - 2.0 * isNeg[i]) * (abs(maxDiff - diffs[i]) < _epsilon ? 1.0 : 0.0);

    if (deb) std::cout << side << ": (";
    if (deb) std::cout << vecStart[0] << " " << vecStart[1] << " " << vecStart[2] << ") + (";

    vec3 intersection;
    for (int i = 0; i < 3; ++i)
    {
        int otherCoord1 = (i == 0) ? 1 : 0;
        int otherCoord2 = (i == 2) ? 1 : 2;
        intersection[i] = (abs(result[i]) != 0) ? 
            ((isNeg[i] == 0 ? (side + paral[i]) : -(float)paral[i]) - side * vecStart[i]) :
            ((abs(result[otherCoord1]) != 0) ?
                (path[otherCoord1] * dir[i] / dir[otherCoord1]) :
                (path[otherCoord2] * dir[i] / dir[otherCoord2]));
        absPos[i] += intersection[i];
        if (isNeg[i] != 0)
            absPos[i] -= _epsilon;

        finish = finish || ((absPos[i] - _epsilon <= minmax[0] && dir[i] < 0) || (absPos[i] >= minmax[1] && dir[i] > 0));
    }
    if (deb) std::cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << ") = ";
    if (deb) std::cout << absPos[0] << " " << absPos[1] << " " << absPos[2] << "" << std::endl;

    absCoord += intersection / side;

    lastRes = result;
    return result;
}
