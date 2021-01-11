#include "VoxelMapRayTracer.h"

using namespace glm;

VoxelMapRayTracer::VoxelMapRayTracer(const VoxelMapFormat& format, uint32_t chunkLoadRadius, bool alignToFourBytes) :
    _format(format),
    _chunkLoadRadius(chunkLoadRadius),
    _chunkLoadDiameter(chunkLoadRadius * 2 + 1),
    _alignToFourBytes(alignToFourBytes)
{ }

void VoxelMapRayTracer::setMapData(const std::vector<uint8_t>& data)
{
    _mapData = data;
}

void VoxelMapRayTracer::raytraceMap(vec3 rayStart, vec3 rayDir, vec3& absCoord, vec3& normal, vec3& color)
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
        notFinish = !_rayTraceChunk(_getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, absCoord, normal, color);

        if (notFinish)
            curChunkPos += ivec3(_marchAndGetNextDir(rayDir, 1, ivec2(-1, 2), marchFinish, marchAbsPos, lastRes));
    }
}

bool VoxelMapRayTracer::_rayTraceChunk(vec3 rayStart, vec3 rayDir, ivec3 curChunkPos, vec3& absCoord, vec3& normal, vec3& color)
{
    vec3 resultColor = vec3(0, 0, 0);

    uint32_t idx = curChunkPos[2] * _chunkLoadDiameter * _chunkLoadDiameter + curChunkPos[1] * _chunkLoadDiameter + curChunkPos[0];
    uint32_t off = _format.chunkFormat.getSizeInBytes() * idx;
    auto chunkH = _format.unformatChunkHeader(_mapData.data() + off, _alignToFourBytes);
    float sideSteps = chunkH.side;

    vec3 marchPos = rayStart * sideSteps;
    vec3 lastRes = vec3(0);
    bool marchFinish = false;

    uvec3 curVoxPos = ivec3(floor(marchPos));

    uint stepsToTake = 1;

    bool keepTracing = true;

    while (keepTracing)
    {
        glm::uint voxOff = curVoxPos[2] * chunkH.side * chunkH.side + curVoxPos[1] * chunkH.side + curVoxPos[0];
        auto voxel = _format.voxelFormat.unformatVoxel(_mapData.data() + chunkH.voxOffset + voxOff);

        stepsToTake = uint(sideSteps / pow(chunkH.base, voxel.power));

        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (!voxel.isEmpty())
        {
            vec3 entry = _getCurEntryPoint(marchPos, stepsToTake, lastRes);

            vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));

            if (_rayTraceVoxel((int)voxel.type, { voxel.color[0], voxel.color[1], voxel.color[2] }, entry, rayDir, absPos, voxRatio, absCoord, normal, color))
                return true;
        }

        _marchAndGetNextDir(rayDir, stepsToTake, ivec2(0, sideSteps), marchFinish, marchPos, lastRes);

        curVoxPos = ivec3(floor(marchPos));

        keepTracing = !marchFinish;
    }

    return false;
}

bool VoxelMapRayTracer::_rayTraceVoxel(int voxType, vec3 voxColor, vec3 rayStart, vec3 rayDir, vec3 absPos, float voxRatio, vec3& absCoord, vec3& normal, vec3& color)
{
    vec3 dir = rayDir;
    dir = normalize(dir);

    vec3 hit;
    vec3 normal;

    float lenVox = 0;

    if (voxType == 0)
        lenVox = raytrace_cube(rayStart, dir, hit, normal);
    else if (voxType == 1)
        lenVox = raytrace_sphere(rayStart, dir, hit, normal);

    hit *= voxRatio;
    lenVox *= voxRatio;

    normal = normalize(normal);

    vec3 colorCoeffs = { 0,0,0 };

    vec3 color = voxColor;
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

        finish = true;
    }

    return color;
}

vec3 VoxelMapRayTracer::_getVoxelData(int chunkOffset, vec3 pos, uint8_t& voxPower, int& voxType)
{
    uint ptr = chunkOffset;
    uint nLeavesBeforeCurrent = 0;
    uint curPwr = 0;
    uint curLayerLen = 1;
    uint nOffsetBytes = get_dword(ptr, false);
    uint base = get_byte(ptr + 4, false);
    uint power = get_byte(ptr + 5, false);
    uint voxSizeInBytes = get_byte(ptr + 6, false);
    ptr += 7;

    uint leavesOnLayers[MAX_VOX_POW + 1];
    for (uint i = 0; i < power + 1; ++i)
    {
        leavesOnLayers[i] = get_dword(ptr, false);
        ptr += 4;
    }

    uint curPwrLayerPos = 0;

    uint totalWidth = uint(pow(base, power));
    uint zLayerLen = uint(pow(base, DIM - 1));
    uint yRowLen = uint(pow(base, DIM - 2));

    uint bytesForThisLayer = bytesForLayer[base][curPwr];

    bool offsetIsFinal = false;
    while (!offsetIsFinal)
    {
        int val;
        if (bytesForThisLayer == 1)
            val = get_byte(ptr, true);
        else if (bytesForThisLayer == 2)
            val = get_word(ptr, true);
        else
            val = get_dword(ptr, true);

        offsetIsFinal = val < 0;

        val = offsetIsFinal ? (-val - 1) : val;
        nLeavesBeforeCurrent += (offsetIsFinal ? val : leavesOnLayers[curPwr]);

        if (offsetIsFinal)
            break;

        ptr += bytesForThisLayer * uint(curLayerLen - curPwrLayerPos);

        uint curSide = totalWidth / uint(pow(base, curPwr));
        uint sidePart = curSide / base;
        curPwrLayerPos = ((uint(pos[2]) % curSide) / sidePart) * zLayerLen + ((uint(pos[1]) % curSide) / sidePart) * yRowLen + ((uint(pos[0]) % curSide) / sidePart);

        uint sz = uint(pow(base, DIM));
        curLayerLen -= leavesOnLayers[curPwr];
        curLayerLen *= sz;

        curPwr++;
        bytesForThisLayer = bytesForLayer[base][curPwr];
        ptr += bytesForThisLayer * uint(val * sz + curPwrLayerPos);
    }

    ptr = chunkOffset + 4 + 3 + (power + 1) * 4 + nOffsetBytes;
    ptr += voxSizeInBytes * nLeavesBeforeCurrent;

    voxPower = curPwr;

    voxType = get_byte(ptr++, true);
    uint color = get_byte(ptr, false);
    float r = float(32 * ((color & 0xE0) >> 5)) / 255.0;
    float g = float(32 * ((color & 0x1C) >> 2)) / 255.0;
    float b = float(64 * (color & 0x03)) / 255.0;

    return vec3(r, g, b);
}

vec3 VoxelMapRayTracer::_getCurEntryPoint(vec3 absPos, float side, vec3 lastRes)
{
    vec3 result = vec3(0, 0, 0);

    vec3 curPos = absPos / side;

    for (int i = 0; i < 3; ++i)
    {
        while (curPos[i] < 0) curPos[i] += 1.0;
        result[i] = (lastRes[i] < 0) ? 1.0 - EPSILON : (curPos[i] - floor(curPos[i]));
    }

    return result;
}

vec3 VoxelMapRayTracer::_marchAndGetNextDir(vec3 dir, float side, ivec2 minmax, bool& finish, vec3& absPos, vec3& lastRes, float& len)
{
    vec3 isNeg;
    for (int i = 0; i < 3; ++i)
        isNeg[i] = dir[i] < 0 ? 1.0 : 0.0;

    vec3 vecStart = getCurEntryPoint(absPos, side, lastRes);

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        path[i] = (isNeg[i] != 0) ? -vecStart[i] : (1.0 - vecStart[i]);
        path[i] = (path[i] == 0) ? EPSILON : path[i];
    }

    vec3 diffs = dir / path;
    float maxDiff = max(diffs.x, max(diffs.y, diffs.z));

    vec3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = (1.0 - 2.0 * isNeg[i]) * (abs(maxDiff - diffs[i]) < EPSILON ? 1.0 : 0.0);

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
            absPos[i] -= EPSILON;

        finish = finish || ((absPos[i] - EPSILON <= minmax[0] && dir[i] < 0) || (absPos[i] >= minmax[1] && dir[i] > 0));
    }

    len += length(intersection);

    lastRes = result;
    return result;
}
