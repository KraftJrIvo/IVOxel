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

void VoxelMapRayTracer::setLightData(const std::vector<uint8_t>& data)
{
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

    if (deb) std::cout << "START " << light << "\n";

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

    if (marchFinish) {
        color = { 255, 0, 0 };
        marchAbsPos = clamp(rayDir * 1000000.0f, vec3(-(float)_chunkLoadRadius), vec3(_chunkLoadRadius + 1.0f));
    }

    if (!light) 
    {
        _drawLights(rayStart, rayDir, marchAbsPos, color);
    }

    if (deb) std::cout << "END" << marchAbsPos.x << " " << marchAbsPos.y << " " << marchAbsPos.z << " : " << color.x << " " << color.y << " " << color.z << " " << "\n";
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
        {
            if (deb) std::cout << "bounds" << "\n";
            break;
        }

        glm::uint voxOff = chunkH.voxOffset + (curVoxPos[0] * chunkH.side * chunkH.side + curVoxPos[1] * chunkH.side + curVoxPos[2]) * _format.voxelFormat.getSizeInBytes(_alignToFourBytes);
        auto voxelState = _format.getVoxelState(_mapData.data() + voxOff);
        stepsToTake = uint(sideSteps / pow(2, voxelState.size)); //!!!
        vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));

        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (voxelState.full)
        {
            if (deb) std::cout << "bef bef" << curChunkPos.x << " " << curChunkPos.y << " " << curChunkPos.z << " -> " << curVoxPos.x << " " << curVoxPos.y << " " << curVoxPos.z << "\n";
            if (deb) std::cout << "bef " << absPos.x << " " << absPos.y << " " << absPos.z << " -> " << absCoord.x << " " << absCoord.y << " " << absCoord.z << "\n";
            vec3 entry = _getCurEntryPoint(marchPos, stepsToTake, lastRes);
            if (_raytraceVoxel(voxOff, voxelState.neighs, entry, rayDir, absPos, voxRatio, absCoord, normal, color, light))
            {
                if (deb) std::cout << "impact " << entry.x << " " << entry.y << " " << entry.z << " -> " << absCoord.x << " " << absCoord.y << " " << absCoord.z << "\n";
                return true;
            }
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

            for (uint i = 0; i < _nLights; ++i)
            {
                float* light = ((float*)_lightData.data() + i * 8);
                float type = light[0];
                vec3 lCoord = { light[1], light[2], light[3] };
                vec4 lColor = { light[4], light[5], light[6], light[7] };

                if (type == 1) // ambient
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        float res = voxColor[i] * lColor[i];
                        color[i] = std::clamp(color[i] + res, 0.0f, voxColor[i]);
                    }
                }
                else if (type == 2) // global
                {
                    vec3 lightHitPoint, n, c;
                    lightHitPoint = raytraceMap(absRayStart + lCoord * _epsilon * 8.0f , lCoord, n, c, true);
                    if (max(lightHitPoint.x, max(lightHitPoint.y, lightHitPoint.z)) > (float)_chunkLoadRadius + 0.95f ||
                        min(lightHitPoint.x, max(lightHitPoint.y, lightHitPoint.z)) < -(float)_chunkLoadRadius + 0.05f)
                        color = voxel.material->shade(color, voxColor, absRayStart, normal, lCoord, lColor);
                }
                else if (type == 3) // local
                {
                    vec3 dirToLight = lCoord - absRayStart;
                    float lightDist = length(dirToLight);
                    dirToLight = normalize(dirToLight);

                    vec3 lightHitPoint, n, c;
                    lightHitPoint = raytraceMap(lCoord, -dirToLight, n, c, true);

                    float diff = length(lightHitPoint - absRayStart);
                    if (diff < _epsilon * 50.0f) 
                        color = voxel.material->shade(color, voxColor, absRayStart, normal, dirToLight, lColor);
                }
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
        absPos[i] += _epsilon * (1.0 - 2.0 * isNeg[i]);
        finish = finish || ((absPos[i] - _epsilon <= minmax[0] && dir[i] < 0) || (absPos[i] >= minmax[1] && dir[i] > 0));
    }
    if (deb) std::cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << ") = ";
    if (deb) std::cout << absPos[0] << " " << absPos[1] << " " << absPos[2] << "" << std::endl;

    absCoord += intersection / side;

    lastRes = result;
    return result;
}

void VoxelMapRayTracer::_drawLights(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3& absPos, glm::vec3& color) const
{
    auto getNearestLinePt = [](vec3 linePnt, vec3 lineDir, vec3 pt)
    {
        vec3 dir = normalize(lineDir);
        vec3 v = pt - linePnt;
        float d = dot(v, dir);
        return linePnt + dir * d;
    };

    auto getPtLineDist = [&](vec3 linePt, vec3 lineDir, vec3 pt)
    {
        vec3 closestPt = getNearestLinePt(linePt, lineDir, pt);
        return length(pt - closestPt);
    };

    for (uint i = 0; i < _nLights; ++i)
    {
        float* light = ((float*)_lightData.data() + i * 8);
        float type = light[0];
        if (type == 3 || type == 2) //local
        {
            vec3 lCoord = { light[1], light[2], light[3] };
            if (type == 2) lCoord *= 4.0f;
            vec4 lColor = { light[4], light[5], light[6], light[7] };
            vec3 toLight = lCoord - rayStart;
            vec3 toHit = absPos - rayStart;
            float lightLen = length(toLight);
            if (dot(toLight, toHit) > 0 && lightLen < length(toHit))
            {
                float lightDist = getPtLineDist(rayStart, rayDir, lCoord);
                float g = (0.1 - lightDist) * 10.0;
                float coeff = g < 0 ? 0 : g;
                vec3 c = coeff * lColor;
                color += c;
            }
        }
    }
}