#include "VoxelMapRayTracer.h"

using namespace glm;

VoxelMapRayTracer::VoxelMapRayTracer(const VoxelMapFormat& format, VoxelTypeStorer& vts, uint32_t chunkLoadRadius, float epsilon, bool alignToFourBytes) :
    _vts(vts),
    _format(format),
    _chunkLoadRadius(chunkLoadRadius),
    _chunkLoadDiameter(chunkLoadRadius * 2 + 1),
    _epsilon(epsilon),
    _alignToFourBytes(alignToFourBytes)
{ }

void VoxelMapRayTracer::setMapData(const std::vector<uint8_t>& data)
{
    _mapData = data;
}

void VoxelMapRayTracer::setLightData(const std::vector<uint8_t>& data)
{
    _lightData = data;
}

void VoxelMapRayTracer::raytraceMap(vec3 rayStart, vec3 rayDir, glm::vec3& absCoord, vec3& normal, vec3& color)
{
    vec3 resultColor = vec3(0, 0, 0);

    vec3 curPos = rayStart;
    ivec3 curChunkPos = ivec3(floor(curPos));

    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;
    vec3 lastRes = vec3(0, 0, 0);

    while (notFinish)
    {
        uint32_t idx = curChunkPos[2] * _chunkLoadDiameter * _chunkLoadDiameter + curChunkPos[1] * _chunkLoadDiameter + curChunkPos[0];
        uint32_t off = _format.chunkFormat.getSizeInBytes() * idx;
        auto chunkH = _format.getChunkState(_mapData.data() + off, _alignToFourBytes);

        vec3 prevAbsCoord = absCoord;
        notFinish = !_rayTraceChunk(chunkH, _getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, absCoord, normal, color);

        if (notFinish)
        {
            absCoord = prevAbsCoord;

            chunkH.parals.data();
            curChunkPos += ivec3(_marchAndGetNextDir(rayDir, 1, ivec2(-1, 2), chunkH.parals.data(), marchFinish, marchAbsPos, lastRes, absCoord));
        }
    }
}

bool VoxelMapRayTracer::_rayTraceChunk(const VoxelChunkState& chunkH, vec3 rayStart, vec3 rayDir, ivec3 curChunkPos, vec3& absCoord, vec3& normal, vec3& color)
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
        glm::uint voxOff = chunkH.voxOffset + curVoxPos[2] * chunkH.side * chunkH.side + curVoxPos[1] * chunkH.side + curVoxPos[0];
        //auto voxel = _format.voxelFormat.unformatVoxel(_vts, _mapData.data() + voxOff);
        auto voxelState = _format.getVoxelState(_mapData.data() + voxOff);

        stepsToTake = uint(sideSteps / voxelState.size);

        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (voxelState.full)
        {
            vec3 entry = _getCurEntryPoint(marchPos, stepsToTake, lastRes);

            vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));

            if (_rayTraceVoxel(voxOff, voxelState.neighs, entry, rayDir, absPos, voxRatio, absCoord, normal, color))
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

bool VoxelMapRayTracer::_rayTraceVoxel(glm::uint voxOff, const VoxelNeighbours& neighs, vec3 rayStart, vec3 rayDir, vec3 absPos, float voxRatio, vec3& absCoord, vec3& normal, vec3& color)
{
    Voxel voxel = _format.voxelFormat.unformatVoxel(_vts, _mapData.data() + voxOff);

    vec3 dir = rayDir;
    dir = normalize(dir);

    vec3 hit;
    vec3 normal;

    float lenVox = 0;

    voxel.shape->raytrace(rayStart, dir, neighs, hit, normal);

    hit *= voxRatio;
    lenVox *= voxRatio;

    normal = normalize(normal);

    vec3 colorCoeffs = { 0,0,0 };

    color = voxel.color;
    vec3 absRayStart = absPos + hit;

    if (lenVox >= 0)
    {
        for (uint i = 0; i <= light.nLights; ++i)
        {
            vec3 lCoords, lColor;
            if (i == light.nLights)
            {
                lCoords = absRayStart + vec3(0.1, 10.1, 0.1);
                lColor = vec3(0.33, 0.33, 0.33);
            }
            else
            {
                lCoords = light.coords[i].xyz;
                lColor = light.colors[i].xyz;
            }

            vec3 dirToLight = lCoords - absRayStart;
            float lightDist = length(dirToLight);
            dirToLight = normalize(dirToLight);

            int b = 0;
            float traceLen = 0;
            raytraceMap2(lCoords, -dirToLight, b, traceLen);

            float lightStr = 0;
            if (traceLen + 0.001 >= lightDist)
            {
                float dotVal = dot(dirToLight, normal);
                lightStr = (dotVal < 0) ? 0 : dotVal;
            }

            colorCoeffs += lColor * lightStr;
        }

        for (uint i = 0; i < 3; ++i)
        {
            colorCoeffs[i] = (colorCoeffs[i] > 1.0) ? 1.0 : (colorCoeffs[i] < 0.0) ? 0 : colorCoeffs[i];
            color[i] = color[i] * colorCoeffs[i];
        }

        len += lenVox;

        return true;
    }

    return false;
}

vec3 VoxelMapRayTracer::_getCurEntryPoint(vec3 absPos, float side, vec3 lastRes)
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

vec3 VoxelMapRayTracer::_marchAndGetNextDir(vec3 dir, float side, ivec2 minmax, glm::uvec3* parals, bool& finish, vec3& absPos, vec3& lastRes, glm::vec3& absCoord)
{
    vec3 isNeg;
    for (int i = 0; i < 3; ++i)
        isNeg[i] = dir[i] < 0 ? 1.0 : 0.0;

    vec3 vecStart = _getCurEntryPoint(absPos, side, lastRes);

    int paralN = (vec3(4.0, 2.0, 1.0) * isNeg).length();
    uvec3 paral = parals[paralN];

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        path[i] = (isNeg[i] != 0) ? -vecStart[i] : (paral[i] - vecStart[i]);
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
