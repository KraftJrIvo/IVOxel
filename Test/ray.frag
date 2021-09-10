#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform CameraData {
    mat4 mvp;
    vec2 res;
    float fov;
    vec3 pos;
    vec3 abspos;
} cam;

layout(set = 1, binding = 0) uniform LightData {
    vec4 data[96 * 2];
} light;

const int VOX_SIZE = 8;
const int CHUNK_SIZE = 8;

const int LOAD_RADIUS = 0;
const int MAX_LIGHTS = 1;
const int EPSILON = 2;
layout(set = 2, binding = 0) uniform ConstData {
    vec4 data;
} constants;

layout(std140, set = 3, binding = 0) readonly buffer MapData {
    uvec4 data[];
} map;

//---UTILS-------------------------------------------------------------------------------------------------------

uint get_byte_group(uint start_byte_ix)
{
  uint uint_in_vec = (start_byte_ix / 4) % 4;
  uint vec_ix = start_byte_ix / 16;

  return map.data[vec_ix][uint_in_vec];
}

uint get_byte(uint byte_ix)
{
  uint byte_in_uint = byte_ix % 4;
  uint bytes = get_byte_group(byte_ix);

  return (bytes >> ((byte_in_uint) * 8)) & 0xFF;
}

uint get_uint(uint byte_ix)
{
    return (get_byte(byte_ix)) | (get_byte(byte_ix + 1) << 0x8) | (get_byte(byte_ix + 2) << 0x10) | (get_byte(byte_ix + 3) << 0x18);
}

//---MAP-UTILS---------------------------------------------------------------------------------------------------

bool checkMapBounds(vec3 absPos)
{
    float chunkLoadRadius = constants.data[LOAD_RADIUS];
    for (int i = 0; i < 3; ++i)
        if (absPos[i] < -chunkLoadRadius || absPos[i] >= chunkLoadRadius + 1.0)
            return true;
    return false;
}

bool checkChunkBounds(vec3 pos, float steps)
{
    for (int i = 0; i < 3; ++i)
        if (pos[i] < 0 || pos[i] >= steps)
            return true;
    return false;
}

void getChunkState(uint off, out bool fullness, out uint voxOff, out uint side, out uvec3 parals[8]) 
{
    fullness = get_byte(off) > 0;
    voxOff = get_uint(off + 1);
    side = get_byte(off + 5);
//    for (int i = 0; i < 8; ++i)
//        for (int j = 0; j < 3; ++j)
//            parals[i][j] = get_byte((off + 6) + 3 * i + j);
}

void getVoxelState(uint off, out bool fullness, out uint size, out uint neighs, out uvec3 parals[8]) 
{
    fullness = get_byte(off) > 0;
    size = get_byte(off + 1);
    // shape + material (2b)
    // color (3b)
    neighs = get_byte(off + 7);
//    for (int i = 0; i < 8; ++i)
//        for (int j = 0; j < 3; ++j)
//            parals[i][j] = get_byte((off + 8) + 3 * i + j);
}

void unformatVoxel(uint off, out uint size, out uint shape, out uint material, out vec3 color)
{
    size = get_byte(off + 1);
    shape = get_byte(off + 2);
    material = get_byte(off + 3);
    color.r = get_byte(off + 4);
    color.g = get_byte(off + 5);
    color.b = get_byte(off + 6);
}

void getLight(uint off, out uint type, out vec3 coord, out vec4 color)
{
    vec4 l1 = light.data[2 * off];
    type = uint(l1[0]);
    coord = l1.yzw;
    color = light.data[2 * off + 1];
}

//---TRACE-UTILS-------------------------------------------------------------------------------------------------

vec3 getCurEntryPoint(vec3 absPos, float side, vec3 lastRes)
{
    float epsilon = constants.data[EPSILON];
    vec3 result = vec3(0);
    vec3 curPos = absPos / side;

    for (int i = 0; i < 3; ++i)
    {
        while (curPos[i] < 0) curPos[i] += 1.0;
        result[i] = (lastRes[i] < 0) ? 1.0 - epsilon : (curPos[i] - floor(curPos[i]));
    }

    return result;
}

vec3 marchAndGetNextDir(vec3 dir, float side, ivec2 minmax, uvec3 parals[8], inout bool finish, inout vec3 absPos, inout vec3 lastRes, inout vec3 absCoord)
{
    float epsilon = constants.data[EPSILON];

    vec3 isNeg;
    for (int i = 0; i < 3; ++i)
        isNeg[i] = dir[i] < 0 ? 1.0 : 0.0;

    vec3 vecStart = getCurEntryPoint(absPos, side, lastRes);

    vec3 vec = vec3(1.0, 2.0, 4.0) * (vec3(1,1,1) - isNeg);
    int paralN = int(vec.x + vec.y + vec.z);
    vec3 paral = parals[paralN];

    vec3 path;
    for (int i = 0; i < 3; ++i)
    {
        if (side > 1 && paral[i] > 0) paral[i] -= side - 1.0;
        path[i] = (isNeg[i] != 0) ? (-side * vecStart[i] - float(paral[i])) : (paral[i] + side * (1.0 - vecStart[i]));
        path[i] = (path[i] == 0) ? epsilon : path[i];
    }

    vec3 diffs = dir / path;
    float maxDiff = max(diffs.x, max(diffs.y, diffs.z));

    vec3 result;
    for (int i = 0; i < 3; ++i)
        result[i] = (1.0 - 2.0 * isNeg[i]) * (abs(maxDiff - diffs[i]) < epsilon ? 1.0 : 0.0);

    vec3 intersection;
    for (int i = 0; i < 3; ++i)
    {
        int otherCoord1 = (i == 0) ? 1 : 0;
        int otherCoord2 = (i == 2) ? 1 : 2;
        intersection[i] = (abs(result[i]) != 0) ? 
            ((isNeg[i] == 0 ? (side + paral[i]) : -float(paral[i])) - side * vecStart[i]) :
            ((abs(result[otherCoord1]) != 0) ?
                (path[otherCoord1] * dir[i] / dir[otherCoord1]) :
                (path[otherCoord2] * dir[i] / dir[otherCoord2]));
        absPos[i] += intersection[i];
        absPos[i] += epsilon * (1.0 - 2.0 * isNeg[i]);
        finish = finish || ((absPos[i] - epsilon <= minmax[0] && dir[i] < 0) || (absPos[i] >= minmax[1] && dir[i] > 0));
    } 

    absCoord += intersection / side;

    lastRes = result;
    return result;
}

bool raytrace_cube(vec3 orig, vec3 dir, out vec3 hit, out vec3 normal)
{
	vec3 g = orig - vec3(0.5f, 0.5f, 0.5f);
	vec3 gabs = vec3(abs(g[0]), abs(g[1]), abs(g[2]));
	float maxG = max(gabs[0], max(gabs[1], gabs[2]));
	for (uint i = 0; i < 3; ++i)
		if (gabs[i] == maxG)
		{
			normal[i] = g[i] / 0.5f;
			normal[(i + 1) % 3] = 0;
			normal[(i + 2) % 3] = 0;
			break;
		}
	hit = orig;

	return true;
}

bool solveQuadratic(float a, float b, float c, out float x0, out float x1)
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

float getSphereIntersectionDist(vec3 orig, vec3 dir)
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

bool raytrace_sphere(vec3 orig, vec3 dir, out vec3 hit, out vec3 normal)
{
    float len = getSphereIntersectionDist(orig, dir);
	hit = orig + dir * len;
	vec3 center = { 0.5f, 0.5f, 0.5f };
	normal = hit - center;

	return len >= 0;
}

bool raytrace(uint type, vec3 start, vec3 dir, uint neighs, out vec3 hit, out vec3 normal)
{
    if (type == 1) 
    {
        return raytrace_cube(start, dir, hit, normal);
    }
    else
    {
        return raytrace_sphere(start, dir, hit, normal);
    }
}

vec3 shade(vec3 curColor, vec3 voxColor, vec3 hitPoint, vec3 normal, vec3 lightDir, vec3 lightColor)
{
	float dotVal = dot(normalize(lightDir), normal);
	float lightStr = (dotVal < 0) ? 0 : dotVal;

	for (int i = 0; i < 3; ++i)
	{
		float res = voxColor[i] * lightColor[i] * lightStr;
		curColor[i] = clamp(curColor[i] + res, 0.0f, voxColor[i]);
	}

	return curColor;
}

vec3 getNearestLinePt(vec3 linePnt, vec3 lineDir, vec3 pt)
{
    vec3 dir = normalize(lineDir);
    vec3 v = pt - linePnt;
    float d = dot(v, dir);
    return linePnt + dir * d;
}

float getPtLineDist(vec3 linePt, vec3 lineDir, vec3 pt)
{
    vec3 closestPt = getNearestLinePt(linePt, lineDir, pt);
    return length(pt - closestPt);
}

void drawLights(vec3 rayStart, vec3 rayDir, vec3 absPos, inout vec3 color)
{
    float maxLights = constants.data[MAX_LIGHTS];

    for (int i = 0; i < maxLights; ++i)
    {
        uint type;
        vec3 lCoord;
        vec4 lColor;
        getLight(i, type, lCoord, lColor);
        if (type == 3) //local
        {
            if (type == 2) lCoord *= 4.0f;
            vec3 toLight = lCoord - rayStart;
            vec3 toHit = absPos - rayStart;
            float lightLen = length(toLight);
            if (dot(toLight, toHit) > 0 && lightLen < length(toHit))
            {
                float lightDist = getPtLineDist(rayStart, rayDir, lCoord);
                float g = (0.1 - lightDist) * 10.0;
                float coeff = g < 0 ? 0 : g;
                color += (coeff * lColor.xyz) * (vec3(1.0) - color) * vec3(max(1.0 - length(lCoord - rayStart) / constants.data[LOAD_RADIUS], 0));
            }
        }
    }
}

//---LIGHT-TRACE--------------------------------------------------------------------------------------------------------

bool raytraceVoxel_light(uint voxOff, const uint neighs, vec3 rayStart, vec3 rayDir, vec3 absPos, float voxRatio, inout vec3 absCoord)
{
    float chunkLoadRadius = constants.data[LOAD_RADIUS];
    float maxLights = constants.data[MAX_LIGHTS];
    float epsilon = constants.data[EPSILON];
    
    uint vox_size;
    uint vox_shape; 
    uint vox_material;
    vec3 vox_color;
    unformatVoxel(voxOff, vox_size, vox_shape, vox_material, vox_color);

    if (vox_shape == 0)
        return false;
    
    vec3 dir = rayDir;
    dir = normalize(dir);
    
    vec3 hitCoord, n;
    bool hit = raytrace(vox_shape, rayStart, dir, neighs, hitCoord, n);
    hitCoord *= voxRatio;

    if (hit)
        absCoord = absPos + hitCoord;
        
    return hit;
}

bool raytraceChunk_light(bool fullness, uint voxOffset, uint side, vec3 rayStart, vec3 rayDir, ivec3 curChunkPos, inout vec3 absCoord)
{
    float sideSteps = side;
    vec3 marchPos = rayStart * sideSteps;
    vec3 lastRes = vec3(0);
    bool marchFinish = false;
    uvec3 curVoxPos = ivec3(floor(marchPos));
    uint stepsToTake = 1;
    bool keepTracing = fullness;
    
    while (keepTracing)
    {
        if (checkChunkBounds(curVoxPos, sideSteps))
            break;

        uint voxOff = voxOffset + (curVoxPos[0] * side * side + curVoxPos[1] * side + curVoxPos[2]) * VOX_SIZE;

        bool vox_fullness;
        uint vox_size;
        uint vox_neighs;
        uvec3 vox_parals[8];
        getVoxelState(voxOff, vox_fullness, vox_size, vox_neighs, vox_parals);

        stepsToTake = (vox_size > 1) ? 1 : uint(sideSteps / pow(2, vox_size));
        vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));

        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (vox_fullness)
        {
            vec3 entry = getCurEntryPoint(marchPos, stepsToTake, lastRes);
            if (raytraceVoxel_light(voxOff, vox_neighs, entry, rayDir, absPos, voxRatio, absCoord))
            return true;
        }
        vec3 absCoordVox = {0,0,0};
        marchAndGetNextDir(rayDir, stepsToTake, ivec2(0, sideSteps), vox_parals, marchFinish, marchPos, lastRes, absCoordVox);
        absCoord += absCoordVox * voxRatio;
        curVoxPos = ivec3(floor(marchPos));
        keepTracing = !marchFinish;
    }

    return false;
}

vec3 raytraceMap_light(vec3 rayStart, vec3 rayDir)
{
    float chunkLoadRadius = constants.data[LOAD_RADIUS];
    float chunkLoadDiameter = chunkLoadRadius * 2.0 + 1.0;

    vec3 lastRes = vec3(0);
    
    ivec3 curChunkPos = ivec3(floor(rayStart));
    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;

    while (notFinish && !marchFinish)
    {
        if (checkMapBounds(curChunkPos))
            break;

        uint idx = uint((curChunkPos.x + chunkLoadRadius) * chunkLoadDiameter * chunkLoadDiameter + (curChunkPos.y + chunkLoadRadius) * chunkLoadDiameter + curChunkPos.z + chunkLoadRadius);
        uint off = CHUNK_SIZE * idx;

	    bool fullness;
	    uint voxOff;
	    uint side;
	    uvec3 parals[8];
        getChunkState(off, fullness, voxOff, side, parals);

        vec3 prevHitPoint = marchAbsPos;
        notFinish = !fullness || !raytraceChunk_light(fullness, voxOff, side, getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, marchAbsPos);

        if (notFinish)
        {
            marchAbsPos = prevHitPoint;
            vec3 coord;
            marchAndGetNextDir(rayDir, 1, ivec2(-chunkLoadRadius, chunkLoadRadius + 1), parals, marchFinish, marchAbsPos, lastRes, coord);
            curChunkPos = ivec3(floor(marchAbsPos));
        }
    }

    if (marchFinish) {
        marchAbsPos = clamp(rayDir * 1000000.0, vec3(-chunkLoadRadius), vec3(chunkLoadRadius + 1.0));
    }

    return marchAbsPos;
}

//---COLOR-TRACE--------------------------------------------------------------------------------------------------------

bool raytraceVoxel(uint voxOff, const uint neighs, vec3 rayStart, vec3 rayDir, vec3 absPos, float voxRatio, inout vec3 absCoord, inout vec3 normal, inout vec3 color)
{
    float chunkLoadRadius = constants.data[LOAD_RADIUS];
    float maxLights = constants.data[MAX_LIGHTS];
    float epsilon = constants.data[EPSILON];
    
    uint vox_size;
    uint vox_shape; 
    uint vox_material;
    vec3 vox_color;
    unformatVoxel(voxOff, vox_size, vox_shape, vox_material, vox_color);

    if (vox_shape == 0)
        return false;
    
    vec3 dir = rayDir;
    dir = normalize(dir);
    
    vec3 hitCoord;
    bool hit = raytrace(vox_shape, rayStart, dir, neighs, hitCoord, normal);
    hitCoord *= voxRatio;
    normal = normalize(normal);

    if (hit)
    {
        vec3 absRayStart = absPos + hitCoord;
        
        vec3 voxColor = vox_color / 255.0;
        color = vec3(0);

        for (uint i = 0; i < maxLights; ++i)
        {
            uint type;
            vec3 lCoord;
            vec4 lColor;
            getLight(i, type, lCoord, lColor);

            if (type == 1) // ambient
            {
                for (int i = 0; i < 3; ++i)
                {
                    float res = voxColor[i] * lColor[i];
                    color[i] = clamp(color[i] + res, 0.0, voxColor[i]);
                }
            }
            else if (type == 2) // global
            {
                vec3 lightHitPoint;
                lightHitPoint = raytraceMap_light(absRayStart + lCoord * epsilon * 8.0, lCoord);
                if (max(lightHitPoint.x, max(lightHitPoint.y, lightHitPoint.z)) > chunkLoadRadius + 0.95 ||
                    min(lightHitPoint.x, max(lightHitPoint.y, lightHitPoint.z)) < -chunkLoadRadius + 0.05)
                    color = shade(color, voxColor, absRayStart, normal, lCoord, lColor.xyz);
            }
            else if (type == 3) // local
            {
                vec3 dirToLight = lCoord - absRayStart;
                float lightDist = length(dirToLight);
                float distCoeff = (1.0 - (lightDist / 1.5));
                
                if (lightDist > 1.5)
                    continue;
                
                dirToLight = normalize(dirToLight);
                
                vec3 lightHitPoint, n, c;
                lightHitPoint = raytraceMap_light(lCoord, -dirToLight);
                
                float diff = length(lightHitPoint - absRayStart);
                if (diff < epsilon * 50.0)
                    color = shade(color, voxColor, absRayStart, normal, dirToLight, lColor.xyz * distCoeff);
            }
            else
                break;
        }
        
        absCoord = absRayStart;
        
        return true;
    }
    return false;
}

bool raytraceChunk(bool fullness, uint voxOffset, uint side, vec3 rayStart, vec3 rayDir, ivec3 curChunkPos, inout vec3 absCoord, inout vec3 normal, inout vec3 color)
{
    vec3 resultColor = vec3(0, 0, 0);
    float sideSteps = side;
    vec3 marchPos = rayStart * sideSteps;
    vec3 lastRes = vec3(0);
    bool marchFinish = false;
    uvec3 curVoxPos = ivec3(floor(marchPos));
    uint stepsToTake = 1;
    bool keepTracing = fullness;
    
    while (keepTracing)
    {
        if (checkChunkBounds(curVoxPos, sideSteps))
            break;

        uint voxOff = voxOffset + (curVoxPos[0] * side * side + curVoxPos[1] * side + curVoxPos[2]) * VOX_SIZE;

        bool vox_fullness;
        uint vox_size;
        uint vox_neighs;
        uvec3 vox_parals[8];
        getVoxelState(voxOff, vox_fullness, vox_size, vox_neighs, vox_parals);

        stepsToTake = (vox_size > 1) ? 1 : uint(sideSteps / pow(2, vox_size));
        vec3 absPos = vec3(curChunkPos) + (vec3(curVoxPos - curVoxPos % stepsToTake) / float(sideSteps));
        
        float voxRatio = float(stepsToTake) / float(sideSteps);

        if (vox_fullness)
        {
            vec3 entry = getCurEntryPoint(marchPos, stepsToTake, lastRes);
            if (raytraceVoxel(voxOff, vox_neighs, entry, rayDir, absPos, voxRatio, absCoord, normal, color)) 
            {
                return true;
            }
        }
        vec3 absCoordVox = {0,0,0};
        marchAndGetNextDir(rayDir, stepsToTake, ivec2(0, sideSteps), vox_parals, marchFinish, marchPos, lastRes, absCoordVox);
        absCoord += absCoordVox * voxRatio;
        curVoxPos = ivec3(floor(marchPos));
        keepTracing = !marchFinish;
    }

    return false;
}

vec3 raytraceMap(vec3 rayStart, vec3 rayDir, inout vec3 normal, inout vec3 color)
{
    float chunkLoadRadius = constants.data[LOAD_RADIUS];
    float chunkLoadDiameter = chunkLoadRadius * 2.0 + 1.0;

    vec3 lastRes = vec3(0);
    
    ivec3 curChunkPos = ivec3(floor(rayStart));
    bool notFinish = true;
    bool marchFinish = false;
    vec3 marchAbsPos = rayStart;

    while (notFinish && !marchFinish)
    {
        if (checkMapBounds(curChunkPos))
            break;

        uint idx = uint((curChunkPos.x + chunkLoadRadius) * chunkLoadDiameter * chunkLoadDiameter + (curChunkPos.y + chunkLoadRadius) * chunkLoadDiameter + curChunkPos.z + chunkLoadRadius);
        uint off = CHUNK_SIZE * idx;

	    bool fullness;
	    uint voxOff;
	    uint side;
	    uvec3 parals[8];
        getChunkState(off, fullness, voxOff, side, parals);

        vec3 prevHitPoint = marchAbsPos;
        notFinish = !fullness || !raytraceChunk(fullness, voxOff, side, getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, marchAbsPos, normal, color);

        if (notFinish)
        {
            marchAbsPos = prevHitPoint;
            vec3 coord;
            marchAndGetNextDir(rayDir, 1, ivec2(-chunkLoadRadius, chunkLoadRadius + 1), parals, marchFinish, marchAbsPos, lastRes, coord);
            curChunkPos = ivec3(floor(marchAbsPos));
        }
    }

    if (marchFinish) {
        marchAbsPos = clamp(rayDir * 1000000.0, vec3(-chunkLoadRadius), vec3(chunkLoadRadius + 1.0));
    }

    color *= vec3(max(1.0 - length(marchAbsPos - rayStart) / chunkLoadRadius, 0));

    drawLights(rayStart, rayDir, marchAbsPos, color);

    return marchAbsPos;
}

//---MAIN--------------------------------------------------------------------------------------------------------

void main()
{
    vec2 coeffs = (gl_FragCoord.xy - cam.res / 2.0) / cam.res.y;
    vec3 coords = vec3(coeffs.y, coeffs.x, -1.0);
    vec3 start = cam.abspos;
    vec3 dir = mat3(cam.mvp) * coords;

    vec3 normal;
    outColor = vec4(vec3(0.0), 1.0);
    raytraceMap(start, dir, normal, outColor.xyz);
    outColor = fract(outColor);
}