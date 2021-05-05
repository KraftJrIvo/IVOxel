#include "VoxelMapRayTracer.h"
#include <iostream>

using namespace glm;

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
    vec3 hitPoint = vec3(0, 0, 0);
    vec3 lastRes = vec3(0, 0, 0);
    
    ivec3 curChunkPos = ivec3(floor(rayStart));
    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;

    while (notFinish && !marchFinish)
    {
        if (_checkBounds({ curChunkPos[0], curChunkPos[1], curChunkPos[2] })) {
            //color = { 255, 0, 0 };
            break;
        }

        uint32_t idx = (curChunkPos[2] + _chunkLoadRadius) * _chunkLoadDiameter * _chunkLoadDiameter + (curChunkPos[1] + _chunkLoadRadius) * _chunkLoadDiameter + curChunkPos[0] + _chunkLoadRadius;
        uint32_t off = _format.chunkFormat.getSizeInBytes() * idx;
        auto chunkH = _format.getChunkState(_mapData.data() + off, _alignToFourBytes);

        vec3 prevHitPoint = hitPoint;
        notFinish = !chunkH.fullness || !_raytraceChunk(chunkH, _getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, hitPoint, normal, color, light);

        if (notFinish)
        {
            hitPoint = prevHitPoint;

            chunkH.parals.data();
            curChunkPos += ivec3(_marchAndGetNextDir(rayDir, 1, ivec2(-(int32_t)_chunkLoadRadius, _chunkLoadRadius + 1), chunkH.parals.data(), marchFinish, marchAbsPos, lastRes, hitPoint));
        }
    }

    //if (marchFinish)
    //    color = { 255, 0, 0 };

    return hitPoint;
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
        glm::uint voxOff = chunkH.voxOffset + (curVoxPos[2] * chunkH.side * chunkH.side + curVoxPos[1] * chunkH.side + curVoxPos[0]) * _format.voxelFormat.getSizeInBytes(_alignToFourBytes);
        auto voxelState = _format.getVoxelState(_mapData.data() + voxOff);
        stepsToTake = uint(sideSteps / pow(2, voxelState.size)); //!!!
        vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));
        
        if (_checkBounds(absPos))
            break;

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
        if (!light)
        {
            vec3 colorCoeffs = { 0,0,0 };
            color = voxel.color / 255.0f;
            vec3 absRayStart = absPos + hitCoord;

            for (uint i = 0; i <= _nLights; ++i)
            {
                float* light = ((float*)_lightData.data());

                vec3 lCoord;
                vec4 lColor;
                if (i == _nLights)
                {
                    lCoord = absRayStart + _ambientLightDir;
                    lColor = _ambientLightColor;
                }
                else
                {
                    lCoord = { light[0], light[1], light[2] };
                    lColor = { light[4], light[5], light[6], light[7] };
                }

                vec3 dirToLight = lCoord - absRayStart;
                float lightDist = length(dirToLight);
                dirToLight = normalize(dirToLight);

                vec3 lightHitPoint, c;
                raytraceMap(lCoord, -dirToLight, lightHitPoint, c, true);

                color = voxel.material->shade(color, absRayStart, normal, dirToLight, lColor);
            }
        }
        
        absCoord += hitCoord;
        
        return true;
    }
    return false;
}

bool VoxelMapRayTracer::_checkBounds(glm::vec3 absPos) const
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

    int paralN = (vec3(1.0, 2.0, 4.0) * isNeg).length();
    uvec3 paral = parals[paralN] + uvec3(1, 1, 1);

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        path[i] = (isNeg[i] != 0) ? -vecStart[i] : (/*paral[i]*/1.0f - vecStart[i]); //!!!
        path[i] = (path[i] == 0) ? _epsilon : path[i];
    }

    vec3 diffs = dir / path;
    float maxDiff = max(diffs.x, max(diffs.y, diffs.z));

    vec3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = (1.0 - 2.0 * isNeg[i]) * (abs(maxDiff - diffs[i]) < _epsilon ? 1.0 : 0.0);

    vec3 intersection;
    for (int i = 0; i < 3; ++i)
    {
        int otherCoord1 = (i == 0) ? 1 : 0;
        int otherCoord2 = (i == 2) ? 1 : 2;
        intersection[i] = (result[i] != 0) ?
            ((isNeg[i] == 0 ? 1.0 : 0) - vecStart[i]) :
            ((result[otherCoord1] != 0) ?
                (path[otherCoord1] * dir[i] / dir[otherCoord1]) :
                (path[otherCoord2] * dir[i] / dir[otherCoord2]));
        absPos[i] += intersection[i] * side;
        if (isNeg[i] != 0)
            absPos[i] -= _epsilon;

        finish = finish || ((absPos[i] - _epsilon <= minmax[0] && dir[i] < 0) || (absPos[i] >= minmax[1] && dir[i] > 0));
    }

    absCoord += intersection;

    lastRes = result;
    return result;
}
