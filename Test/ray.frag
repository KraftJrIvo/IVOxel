#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform ViewShaderInfo {
    mat4 mvp;
    vec2 resolution;
    float time;
    float fov;
    vec3 pos;
} view;

layout(set = 1, binding = 0) uniform LightingShaderInfo {
    int nLights;
    float absCoords[16];
    int colors[4 * 16];
} light;

layout(set = 2, binding = 0) uniform MapShaderInfo {
    vec4 chunkOffsets[8];
    int chunks[1024];
} map;

vec3 getCurEntryPoint(vec3 absPos, float side, vec3 lastRes)
{
	vec3 result = vec3(0, 0, 0);

	vec3 curPos = absPos / side;

    for (int i = 0; i < 3; ++i)
    {
        while (curPos[i] < 0) curPos[i] += 1.0;
        result[i] = (lastRes[i] < 0) ? 0.999999 : (curPos[i] - floor(curPos[i]));
    }

	return result;
}

vec3 marchAndGetNextDir(vec3 dir, vec2 minMaxX, vec2 minMaxY, vec2 minMaxZ, float side, inout bool finish, inout vec3 absPos, inout vec3 lastRes)
{
    vec3 isNeg;
    for (int i = 0; i < 3; ++i)
        isNeg[i] = dir[i] < 0 ? 1.0 : 0.0;
    
    vec3 vecStart = getCurEntryPoint(absPos, side, lastRes);

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        path[i] = (isNeg[i] != 0) ? -vecStart[i] : (1.0 - vecStart[i]);
        path[i] = (path[i] == 0) ? 0.000001f : path[i];
    }

    vec3 diffs = dir / path;
    float maxDiff = max(diffs.x, max(diffs.y, diffs.z));

    vec3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = (1.0 - 2.0 * isNeg[i]) * (abs(maxDiff - diffs[i]) < 0.000001f ? 1.0 : 0.0);

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
            absPos[i] -= 0.00001;

        finish = finish || ((absPos[i] < -1 && dir[i] < 0) || (absPos[i] > 2 && dir[i] > 0));
    }

    lastRes = result;
	return result;
}

int getChunkOffset(vec3 pos)
{
    if (pos.x >= -1 && pos.x <= 1 && pos.y >= -1 && pos.y <= 1 && pos.z >= -1 && pos.z <= 1)
    {
        int id = int(pos.z + 1) * 9 + int(pos.y + 1) * 3 + int(pos.x + 1);
        int vecId = id / 4;
        int elemId = id % 4;
        return int(map.chunkOffsets[vecId][elemId]);
    }
    return -1;
}

vec3 _raytraceMap(vec3 rayStart, vec3 rayDir, inout int bounces)
{
	vec3 resultColor = vec3(0, 0, 0);

    vec3 curPos = rayStart;
    vec3 curChunkPos = floor(curPos);

    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;
    vec3 lastRes = vec3(0,0,0);

    const vec2 minmax = vec2(-1.0, 1.0);

    int it = 0;

    while (notFinish)
	{
        int chunkOffset = getChunkOffset(curChunkPos);

        if (chunkOffset != -1 && it > 0) 
        {
            resultColor = (curChunkPos + 1.0) / 2.0;
            marchFinish = true;
        }

        notFinish = bounces > 0 && !marchFinish;

        if (notFinish)
            curChunkPos += marchAndGetNextDir(rayDir, minmax, minmax, minmax, 1, marchFinish, marchAbsPos, lastRes);

        it++;
    }
	
	return resultColor;
}

void main()
{
    vec2 coeffs = (gl_FragCoord.xy - view.resolution.xy/2.0)/view.resolution.y;

    vec3 coords = vec3(coeffs.y, -coeffs.x, -1.0);

    vec3 start =  view.pos;
    vec3 dir = mat3(view.mvp) * coords;

    int bounces = 1;
    outColor = vec4(_raytraceMap(start, dir, bounces), 1.0);
}