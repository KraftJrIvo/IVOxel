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
    uvec4 chunks[4096 / 16];
} map;

uint get_byte(uint byte_ix)
{
  uint byte_in_uint = byte_ix % 4;
  uint uint_in_vec = (byte_ix / 4) % 4;
  uint vec_ix = byte_ix / 16;

  uint bytes = map.chunks[vec_ix][uint_in_vec];
  return (bytes >> ((4 - byte_in_uint) * 8)) & 0xFF; //Little-endian. For Big-endian, remove the "4 -" part.
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

//Voxel CPURenderer::getVoxelData(const VoxelMap& map, const VoxelPyramid& pyram, const std::vector<uint32_t>& pos) const
//{
//	Voxel vox;
//
//	uint32_t nLeavesBeforeCurrent = 0;
//	uint32_t curPwr = 0;
//	uint32_t curLayerLen = 1;
//	std::vector<uint32_t> leavesOnLayers;
//	uint32_t nOffsetBytes = *((uint32_t*)pyram.data.data());
//	uint8_t voxSizeInBytes = pyram.data[4];
//	uint8_t* ptr = (uint8_t*)pyram.data.data() + sizeof(uint32_t) + sizeof(uint8_t);
//
//	for (uint8_t i = 0; i < pyram.power + 1; ++i)
//	{
//		leavesOnLayers.push_back(*((uint32_t*)ptr));
//		ptr += sizeof(uint32_t);
//	}
//
//	uint32_t curPwrLayerPos = 0;
//
//	uint32_t totalWidth = std::pow(pyram.base, pyram.power);
//	uint32_t zLayerLen = std::pow(pyram.base, DIMENSIONS - 1);
//	uint32_t yRowLen = std::pow(pyram.base, DIMENSIONS - 2);
//
//	uint8_t bytesForThisLayer = VoxelPyramid::getPyramLayerBytesCount(pyram.base, curPwr);
//
//	bool offsetIsFinal = false;
//	while (!offsetIsFinal)
//	{
//		int8_t* offset = (int8_t*)ptr;
//		int32_t val;
//
//		if (bytesForThisLayer == 1)
//			val = *(offset);
//		else if (bytesForThisLayer == 2)
//			val = *((int16_t*)offset);
//		else
//			val = *((int32_t*)offset);
//
//		bool offsetIsFinal = val < 0;
//		
//		val = offsetIsFinal ? (-val - 1) : val;
//		nLeavesBeforeCurrent += (offsetIsFinal ? val : leavesOnLayers[curPwr]);
//
//		if (offsetIsFinal)
//			break;
//
//		ptr += bytesForThisLayer * uint32_t(curLayerLen - curPwrLayerPos); // skipping to the end of current layer
//
//		uint32_t curSide = totalWidth / std::pow(pyram.base, curPwr);
//		uint32_t sidePart = curSide / pyram.base;
//		curPwrLayerPos = ((pos[Z] % curSide) / sidePart) * zLayerLen + ((pos[Y] % curSide) / sidePart) * yRowLen + ((pos[X] % curSide) / sidePart);
//
//		curLayerLen -= leavesOnLayers[curPwr];
//		curLayerLen *= std::pow(pyram.base, DIMENSIONS);
//
//		curPwr++;
//		bytesForThisLayer = VoxelPyramid::getPyramLayerBytesCount(pyram.base, curPwr);
//		ptr += bytesForThisLayer * uint32_t(val * std::pow(pyram.base, DIMENSIONS) + curPwrLayerPos); // skipping to the value of interest
//	}
//
//	ptr = (uint8_t*)pyram.data.data() + sizeof(uint32_t) + sizeof(uint8_t) +
//		leavesOnLayers.size() * sizeof(uint32_t) + nOffsetBytes;
//
//	ptr += voxSizeInBytes * nLeavesBeforeCurrent;
//
//	auto voxData = map.getType().unformatVoxelData(ptr);
//	vox.type = std::get<0>(voxData);
//	vox.color = std::get<1>(voxData);
//	vox.neighs = std::get<2>(voxData);
//	vox.power = curPwr;
//
//	return vox;
//}

//std::vector<uint8_t> CPURenderer::_rayTraceVoxel(const VoxelMap& map, const VoxelChunk& chunk, const Voxel& vox, Ray& ray, const std::vector<float>& absPose, float voxSide) const
//{
//	Eigen::Vector3f orig = { ray.start[X], ray.start[Y], ray.start[Z] };
//	Eigen::Vector3f dir = { ray.direction[X], ray.direction[Y], ray.direction[Z] };
//	dir.normalize();
//
//	Eigen::Vector3f hit;
//	Eigen::Vector3f normal;
//
//	if (vox.type == 0)
//		ray.length = Cube::rayTrace(orig, dir, hit, normal);
//	else if (vox.type == 1)
//		ray.length = Sphere::rayTrace(orig, dir, hit, normal);
//
//	normal.normalize();
//
//	float voxDiv = (std::pow(chunk.pyramid.base, chunk.pyramid.power));
//
//	std::vector<float> dists;
//	std::vector<float> colorCoeffs = { 0,0,0 };
//	auto color = vox.color;
//
//	if (!ray.lightRay && ray.length >= 0)
//	{
//		for (auto& lightsInChunk : map.getLightsByChunks())
//		{
//			for (auto& light : lightsInChunk)
//			{
//				Eigen::Vector3f absRayStart = { absPose[X] + voxSide * hit[X]/voxDiv, absPose[Y] + voxSide * hit[Y]/voxDiv, absPose[Z] + voxSide * hit[Z]/voxDiv };
//				Eigen::Vector3f dirToLight = { (light.position[X] - absRayStart[X]), (light.position[Y] - absRayStart[Y]), (light.position[Z] - absRayStart[Z]) };
//				float lightDist = sqrt(dirToLight[X] * dirToLight[X] + dirToLight[Y] * dirToLight[Y] + dirToLight[Z] * dirToLight[Z]);
//				dirToLight.normalize();
//
//				Ray lightRay(1, { light.position[X], light.position[Y], light.position[Z] }, -dirToLight, 1.0f, true);
//				
//				_rayTraceMap(map, lightRay);
//
//				float lightStr = 0;
//				if (lightRay.length + 0.001f >= lightDist)
//				{
//					float dotVal = dirToLight.dot(normal);
//					lightStr = (dotVal < 0) ? 0 : dotVal;
//				}
//
//				for (uint8_t i = 0; i < RGB; ++i)
//					colorCoeffs[i] += (light.rgba[i] / 255.0f) * lightStr;
//			}
//		}
//
//		for (uint8_t i = 0; i < RGB; ++i)
//		{
//			colorCoeffs[i] = (colorCoeffs[i] > 1.0f) ? 1.0f : (colorCoeffs[i] < 0.0f) ? 0 : colorCoeffs[i];
//			color[i] = uint8_t(color[i] * colorCoeffs[i]);
//		}
//	}
//
//	// dbg normal
//	//return { uint8_t(128.0f + normal[X] * 127.0f), uint8_t(128.0f + normal[Y] * 127.0f), uint8_t(128.0f + normal[Z] * 127.0f) };
//
//	// dbg normal + light
//	//return { uint8_t((128.0f + normal[X] * 127.0f) * colorCoeffs[X]), uint8_t((128.0f + normal[Y] * 127.0f) * colorCoeffs[Y]), uint8_t((128.0f + normal[Z] * 127.0f) * colorCoeffs[Z]) };
//
//	return color;
//}

vec3 _rayTraceChunk(int chunkOffset, vec3 rayStart, vec3 rayDir, vec3 curChunkPos, inout int bounces, inout float len)
{
    vec3 resultColor = vec3(0,0,0);

    uint base = get_byte(chunkOffset + 4);
    uint power = get_byte(chunkOffset + 5);

    float sideSteps = pow(base, power);

    vec3 start = rayStart * sideSteps;

    uvec3 curVoxPos = uvec3(floor(rayStart));

    uint stepsToTake = 1;

    bool keepTracing = true;

    while (keepTracing)
    {
    }
//
//	uint32_t stepsToTake = 1;
//
//	bool keepTracing = true;
//
//	while (keepTracing)
//	{
//			vox = getVoxelData(map, chunk.pyramid, curVoxPos);
//			stepsToTake = sideSteps / std::pow(chunk.pyramid.base, vox.power);
//
//			if (vox.type != -1)
//			{
//				//stepsToTake = 1; // for size 1 render
//
//				Ray voxRay = ray;
//				voxRay.start = marcher.getCurEntryPoint(stepsToTake);
//
//				std::vector<float> absPose(3);
//				for (uint8_t i = 0; i < DIMENSIONS; ++i)
//					absPose[i] = curChunkPos[i] + (stepsToTake * float(curVoxPos[i] / stepsToTake)) / sideSteps;
//
//				auto color = _rayTraceVoxel(map, chunk, vox, voxRay, absPose, stepsToTake);
//
//				if (voxRay.length >= 0)
//				{
//					ray.length += utils::calculateDist(ray.start, marcher.getAbsPos(), float(sideSteps));
//					ray.length += (voxRay.length * stepsToTake) / float(sideSteps);
//					ray.bouncesLeft--;
//					return color;
//				}
//			}
//
//			auto ppose = marcher.getAbsPos();
//			marcher.marchAndGetNextDir(minMaxs, stepsToTake);
//				
//			// for dbg
//			//return { ((off[0] != 0) ? (unsigned char)255 : (unsigned char)0), ((off[1] != 0) ? (unsigned char)255 : (unsigned char)0), ((off[2] != 0) ? (unsigned char)255 : (unsigned char)0) };
//				
//			auto pose = marcher.getAbsPos();
//			for (uint8_t i = 0; i < DIMENSIONS; ++i)
//				curVoxPos[i] = floor(pose[i]);
//
//			keepTracing = !marcher.checkFinished();
//	}
//
//	// calculate length in chunks
//	ray.length += utils::calculateDist(ray.start, marcher.getAbsPos(), float(sideSteps));
//
	return resultColor;
}

vec3 _raytraceMap(vec3 rayStart, vec3 rayDir, inout int bounces, inout float len)
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
            //(curChunkPos + 1.0) / 2.0;
            //marchFinish = true;
            len += calculateDist(rayStart, marchAbsPos, 1.0);
            resultColor = _rayTraceChunk(chunkOffset, getCurEntryPoint(marchAbsPos, 1.0, lastRes), rayDir, curChunkPos, bounces, len);
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