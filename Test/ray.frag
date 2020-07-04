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
    float absCoords[3 * 16];
    int colors[4 * 16];
} light;

layout(set = 2, binding = 0) uniform MapShaderInfo {
    vec4 chunkOffsets[8];
    uvec4 chunks[4096 / 16];
} map;

uint get_uint(uint start_byte_ix)
{
  uint uint_in_vec = (start_byte_ix / 4) % 4;
  uint vec_ix = start_byte_ix / 16;

  return map.chunks[vec_ix][uint_in_vec];
}

int get_byte(uint byte_ix, bool signed)
{
  uint byte_in_uint = byte_ix % 4;
  uint bytes = get_uint(byte_ix);

  uint result = (bytes >> ((byte_in_uint) * 8)) & 0xFF;

  if (signed && (result & 0x80) != 0)
    return int(result) - int(0x100);
  return int(result);
}


int get_word(uint byte_ix, bool signed)
{
  uint byte_in_uint = (byte_ix + 1) % 4;
  uint bytes = get_uint(byte_ix);

  uint result = (bytes >> ((byte_in_uint) * 8)) & 0xFFFF;

  if (signed && (result & 0x8000) > 0)
    return int(result) - 0x10000;
  return int(result);
}

int get_dword(uint start_byte_ix, bool signed)
{
  uint byte1 = get_byte(start_byte_ix, false);
  uint byte2 = get_byte(start_byte_ix + 1, false);
  uint byte3 = get_byte(start_byte_ix + 2, false);
  uint byte4 = get_byte(start_byte_ix + 3, false);

  uint result = (byte4 << 8 * 3) + (byte3 << 8 * 2) + (byte2 << 8) + byte1;

  if (signed && (byte4 & 0x80) > 0)
     return int(result) - 0xFFFFFFFF - 1;
  return int(result);
}

float calculateDist(vec3 start, vec3 end, float div)
{
	vec3 pos = end - start;

	float val = length(pos);

	if (div != 1.0f)
		val /= div;

	return val;
}

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

vec3 marchAndGetNextDir(vec3 dir, float side, inout bool finish, inout vec3 absPos, inout vec3 lastRes)
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

const uint MAX_VOX_POW = 5;
const uint DIM = 3;

float raytrace_cube(vec3 orig, vec3 dir, vec3 hit, vec3 normal)
{
	vec3 g = orig - vec3(0.5f, 0.5f, 0.5f);
	vec3 gabs = vec3(abs(g[0]), abs(g[1]), abs(g[2]));
	float maxG = max(gabs[0], max(gabs[1], gabs[2]));
	for (uint i = 0; i < DIM; ++i)
		if (gabs[i] == maxG)
		{
			normal[i] = g[i] / 0.5f;
			normal[(i + 1) % DIM] = 0;
			normal[(i + 2) % DIM] = 0;
			break;
		}
	hit = orig;

	return 0;
}


bool solveQuadratic(float a, float b, float c, inout float x0, inout float x1)
{
	float discr = b * b - 4 * a * c;

	if (discr < 0) 
        return false;

	else if (discr == 0) 
        x0 = x1 = -0.5 * b / a;
	else 
    {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}

	if (x0 > x1) 
    {
        float temp = x0;
        x0 = x1;
        x1 = temp;
    }

	return true;
}

float getIntersectionDist(vec3 orig, vec3 dir)
{
	float t0, t1; // solutions for t if the ray intersects 

	vec3 center = { 0.5, 0.5, 0.5 };
	float radius2 = 0.49 * 0.49;

	vec3 L = orig - center;
	float a = dot(dir, dir);
	float b = 2 * dot(dir, L);
	float c = dot(L, L) - radius2;

	if (!solveQuadratic(a, b, c, t0, t1)) 
        return -1;

	if (t0 > t1) 
    {
        float temp = t0;
        t0 = t1;
        t1 = t0;
    }

	if (t0 < 0) 
    {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) 
            return -1; // both t0 and t1 are negative 
	}

	return t0;
}

float raytrace_sphere(vec3 orig, vec3 dir, inout vec3 hit, inout vec3 normal)
{
    float len = getIntersectionDist(orig, dir);
	hit = orig + dir * len;
	vec3 center = { 0.5f, 0.5f, 0.5f };
	normal = hit - center;

	return len;
}

vec3 rayTraceVoxel(vec4 voxType, vec3 rayStart, vec3 rayDir, vec3 absPos, float chunkSide, float voxSide, inout float len, inout bool finish)
{
    vec3 orig = rayStart;
    vec3 dir = rayDir;
    dir = normalize(dir);

    vec3 hit;
    vec3 normal;

    uint type = uint(voxType[0]) % 10;
    if (type == 0)
        len = raytrace_cube(orig, dir, hit, normal);
    else if (type == 1)
        len = raytrace_sphere(orig, dir, hit, normal);

    normal = normalize(normal);

    vec3 voxColor = {voxType[1] / 255.0, voxType[2] / 255.0, voxType[3] / 255.0};
    vec3 colorCoeffs = {0,0,0};

    vec3 color = voxColor;

    if (len >= 0)
    {
        for (uint i = 0; i < light.nLights; ++i)
        {
            vec3 absRayStart = {absPos[0] + voxSide * hit[0]/chunkSide, absPos[1] + voxSide * hit[1]/chunkSide, absPos[2] + voxSide * hit[2]/chunkSide };
            vec3 dirToLight = {(light.absCoords[3 * i] - absRayStart[0]), (light.absCoords[3 * i + 1] - absRayStart[1]), (light.absCoords[3 * i + 2] - absRayStart[2])};
            float lightDist = length(dirToLight);
            dirToLight = normalize(dirToLight);

            // trace
            len = lightDist;
            // trace

            float lightStr = 0;
            if (len + 0.001 >= lightDist)
			{
				float dotVal = dot(dirToLight, normal);
				lightStr = (dotVal < 0) ? 0 : dotVal;
			}

            for (uint j = 0; j < 3; ++j)
				colorCoeffs[j] += (light.colors[4 * i + j] / 255.0) * lightStr;
        }

        for (uint i = 0; i < 3; ++i)
		{
			colorCoeffs[i] = (colorCoeffs[i] > 1.0) ? 1.0 : (colorCoeffs[i] < 0.0) ? 0 : colorCoeffs[i];
			color[i] = color[i] * colorCoeffs[i];
		}
    }

    return color;
}

#define BYTES_FOR(base, power) ceil(log2(pow(pow(base, power), DIM)) / 8.0f) < 3 ? uint(ceil(log2(pow(pow(base, power), DIM)) / 8.0f)) : 4

const uint bytesForLayer[4][MAX_VOX_POW + 1] = 
{
    {0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 2, 2},
    {1, 1, 2, 2, 4, 4}
};

vec4 getVoxelType(int chunkOffset, vec3 pos)
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

    //return vec4(1.0, leavesOnLayers[0], power > 0 ? leavesOnLayers[1] : 0, power > 1 ? leavesOnLayers[2] : 0);
    
    uint curPwrLayerPos = 0;

    uint totalWidth = uint(pow(base, power));
    uint zLayerLen = uint(pow(base, DIM - 1));
	uint yRowLen = uint(pow(base, DIM - 2));

    uint bytesForThisLayer = bytesForLayer[base][curPwr];

    //return vec4(2.0, base,power, base )/3.0;
    //return vec4(2.0, nOffsetBytes,nOffsetBytes, nOffsetBytes )/10.0;

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

    ptr = chunkOffset + 4 + 3 + (power+1) * 4 + nOffsetBytes;
    ptr += voxSizeInBytes * nLeavesBeforeCurrent;

    uint type = get_byte(ptr++, false) + 10 * power;
    uint color = get_byte(ptr, false);
    float r = (32 * ((color & 0xE0) >> 5)) / 255.0;
    float g = (32 * ((color & 0x1C) >> 2)) / 255.0;
    float b = (64 * (color & 0x03)) / 255.0;

    return vec4(type, r, g, b);
}

vec3 rayTraceChunk(int chunkOffset, vec3 rayStart, vec3 rayDir, vec3 curChunkPos, inout int bounces, inout float len)
{
    vec3 resultColor = vec3(0,0,0);

    uint base = get_byte(chunkOffset + 4, false);
    uint power = get_byte(chunkOffset + 5, false);

    float sideSteps = pow(base, power);

    vec3 start = rayStart * sideSteps;
    vec3 lastRes = vec3(0);
    vec3 marchPos = start;
    bool marchFinish = false;

    vec3 curVoxPos = floor(start);

    uint stepsToTake = 1;

    bool keepTracing = true;

    while (keepTracing)
    {
        vec4 voxType = getVoxelType(chunkOffset, curVoxPos);
        return vec3(voxType[1], voxType[2], voxType[3]);

        uint voxPow = uint(voxType[0])/10;
        stepsToTake = uint(sideSteps / pow(base, voxType[0]/10));

        if (voxType.x != -1)
        {
            vec3 entry = getCurEntryPoint(marchPos, stepsToTake, lastRes);

            vec3 absPos = curChunkPos + (stepsToTake * curVoxPos/stepsToTake)/sideSteps;
            
            bool finish = false;
            vec3 color = rayTraceVoxel(voxType, rayStart, rayDir, absPos, sideSteps, float(stepsToTake), len, finish);

            if (finish)
            {
                len += calculateDist(rayStart, marchPos, sideSteps);
                len += (len * stepsToTake) / float(sideSteps);
                bounces--;
                return color;
            }
        }

        marchAndGetNextDir(rayDir, stepsToTake, marchFinish, marchPos, lastRes);

        curVoxPos = ivec3(floor(marchPos));

        keepTracing = !marchFinish;
    }

    len += calculateDist(rayStart, marchPos, sideSteps);

	return resultColor;
}

vec3 raytraceMap(vec3 rayStart, vec3 rayDir, inout int bounces, inout float len)
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
            len += calculateDist(rayStart, marchAbsPos, 1.0);
            resultColor = rayTraceChunk(chunkOffset, getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, bounces, len);
            //resultColor = curChunkPos;
            break;
        }

        notFinish = bounces > 0 && !marchFinish;

        if (notFinish)
            curChunkPos += marchAndGetNextDir(rayDir, 1, marchFinish, marchAbsPos, lastRes);

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
    float len = 0;
    outColor = vec4(raytraceMap(start, dir, bounces, len), 1.0);
}