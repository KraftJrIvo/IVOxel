#include "CPURenderer.h"

#include <algorithm>

#include <opencv2/videoio.hpp>

#include "Cube.h"
#include "Sphere.h"

#include <omp.h>

CPURenderer::CPURenderer()
{
}

void CPURenderer::render(const VoxelMap& map, Camera& cam) const
{
	while (true)
	{
		cv::Mat result(cam.resolution[1], cam.resolution[0], CV_8UC3, cv::Scalar(0, 0, 0));

		#pragma omp parallel
		{
			uint8_t nThreads = omp_get_num_threads();
			uint8_t threadId = omp_get_thread_num();

			for (uint32_t i = threadId; i < cam.resolution[0]; i += nThreads)
				for (uint32_t j = 0; j < cam.resolution[1]; ++j)
				{
					auto pixel = _renderPixel(map, cam, i, j);
					result.at<cv::Vec3b>(j, i) = { pixel[2], pixel[1], pixel[0] };
				}
			
			#pragma omp barrier
		}

		cv::imshow("IVOxel 1.0", result);
		auto keyCode = cv::waitKey(-1);

		float speed = 0.1f;
		float rspeed = 0.1f;

		if (keyCode == 113) cam.move({0, speed, 0});
		else if (keyCode == 119) cam.move({ 0, 0, speed });
		else if (keyCode == 101) cam.move({ 0, -speed, 0 });
		else if (keyCode == 97) cam.move({ -speed, 0, 0 });
		else if (keyCode == 115) cam.move({ 0, 0, -speed });
		else if (keyCode == 100) cam.move({ speed, 0, 0 });
		else if (keyCode == 56) cam.rotate({ rspeed, 0, 0 });
		else if (keyCode == 52) cam.rotate({ 0, rspeed, 0 });
		else if (keyCode == 54) cam.rotate({ 0, -rspeed, 0 });
		else if (keyCode == 50) cam.rotate({ -rspeed, 0, 0 });
	}
}

void CPURenderer::renderVideo(VoxelMap& map, Camera& cam)
{
	cv::Mat result(cam.resolution[1], cam.resolution[0], CV_8UC3, cv::Scalar(0, 0, 0));

	cv::VideoWriter vid("render.mp4", -1, 60, cv::Size(cam.resolution[0], cam.resolution[1]));
	for (int i = 0; i < 360; ++i)
	{
		std::vector<float> pos(3, 0);
		float angle = 3.14159f * (float(i) / 180.0f);
		pos = { 2.0f * cos(angle), 0.75f, 2.0f * sin(angle) };
		map.moveLight(1, 0, pos);
		for (uint32_t i = 0; i < cam.resolution[0]; ++i)
			for (uint32_t j = 0; j < cam.resolution[1]; ++j)
			{
				auto pixel = _renderPixel(map, cam, i, j);
				result.at<cv::Vec3b>(j, i) = { pixel[2], pixel[1], pixel[0] };
			}
		vid.write(result);
		std::cout << i << "/360\n";
	}
}


std::vector<uint8_t> CPURenderer::_renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y) const
{
	std::vector<uint8_t> rgb = {0,0,0};

	float halfRes = cam.resolution[X] / 2;
	float halfFovRad = (cam.fov / 180.0f) * 3.14159f / 2.0f;

	float coeffX = (float(x) - halfRes) / halfRes;
	float coeffY = -(float(y) - cam.resolution[Y] / 2) / halfRes;

	float coordX = coeffX * sin(halfFovRad);
	float coordY = coeffY * sin(halfFovRad);
	float coordZ = cos(halfFovRad);

	Eigen::Vector3f viewVec(coordX, coordY, coordZ);

	Eigen::Quaternionf quat =
		Eigen::AngleAxisf(cam.rotation[0], Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(cam.rotation[1], Eigen::Vector3f::UnitY())
		* Eigen::AngleAxisf(cam.rotation[2], Eigen::Vector3f::UnitZ());
	viewVec = quat.matrix() * viewVec;

	Ray ray(1, cam.translation, viewVec, 1.0f);

	return _rayTraceMap(map, ray);
}

std::vector<uint8_t> CPURenderer::_rayTraceMap(const VoxelMap& map, Ray& ray) const
{
	std::vector<uint8_t> resultColor = { 0, 0, 0 };

	RayVoxelMarcher marcher;
	std::vector<int32_t> curChunkPos = {
		int32_t(floor(ray.start[X])),
		int32_t(floor(ray.start[Y])),
		int32_t(floor(ray.start[Z]))
	};
	marcher.setStart(ray);

	auto curPos = marcher.getAbsPos();
	bool notFinish = true;

	while (notFinish)
	{
		auto* chunk = map.getChunk(curChunkPos);
		if (chunk)
		{
			Ray entryRay(ray.bouncesLeft, marcher.getCurEntryPoint(), ray.direction, ray.strength, ray.lightRay);
			resultColor = _rayTraceChunk(map, *chunk, entryRay, curChunkPos);
			ray.bouncesLeft = entryRay.bouncesLeft;
			ray.length += utils::calculateDist(ray.start, marcher.getAbsPos());
			ray.length += entryRay.length;
		}

		notFinish = ray.bouncesLeft > 0 && !marcher.checkFinished();

		if (notFinish)
		{
			auto off = marcher.marchAndGetNextDir(map.getMinMaxChunks());

			for (uint8_t i = 0; i < DIMENSIONS; ++i)
				curChunkPos[i] += off[i];
		}
	}

	// depth dbg
	//return {(uint8_t)(ray.length * 64.0f), (uint8_t)(ray.length * 64.0f), (uint8_t)(ray.length * 64.0f) };
	
	return resultColor;
}

std::vector<uint8_t> CPURenderer::_rayTraceChunk(const VoxelMap& map, const VoxelChunk& chunk, Ray& ray, const std::vector<int32_t>& curChunkPos) const
{
	ray.color = { 10, 10, 10 };

	Voxel vox;

	uint32_t sideSteps = std::pow(chunk.pyramid.base, chunk.pyramid.power);
	std::vector<std::pair<int32_t, int32_t>> minMaxs = { { 0, sideSteps }, {0, sideSteps }, {0, sideSteps } };

	ray.start = {
		ray.start[X] * sideSteps,
		ray.start[Y] * sideSteps,
		ray.start[Z] * sideSteps
	};

	RayVoxelMarcher marcher;
	std::vector<uint32_t> curVoxPos = {
		uint32_t(floor(ray.start[X])),
		uint32_t(floor(ray.start[Y])),
		uint32_t(floor(ray.start[Z]))
	};
	marcher.setStart(ray);

	uint32_t stepsToTake = 1;

	bool keepTracing = true;

	while (keepTracing)
	{
			vox = getVoxelData(map, chunk.pyramid, curVoxPos);
			stepsToTake = sideSteps / std::pow(chunk.pyramid.base, vox.power);

			if (vox.type != -1)
			{
				//stepsToTake = 1; // for size 1 render

				Ray voxRay = ray;
				voxRay.start = marcher.getCurEntryPoint(stepsToTake);

				std::vector<float> absPose(3);
				for (uint8_t i = 0; i < DIMENSIONS; ++i)
					absPose[i] = curChunkPos[i] + (stepsToTake * float(curVoxPos[i] / stepsToTake)) / sideSteps;

				auto color = _rayTraceVoxel(map, chunk, vox, voxRay, absPose, stepsToTake);

				if (voxRay.length >= 0)
				{
					ray.length += utils::calculateDist(ray.start, marcher.getAbsPos(), float(sideSteps));
					ray.length += (voxRay.length * stepsToTake) / float(sideSteps);
					ray.bouncesLeft--;
					return color;
				}
			}

			auto ppose = marcher.getAbsPos();
			marcher.marchAndGetNextDir(minMaxs, stepsToTake);
				
			// for dbg
			//return { ((off[0] != 0) ? (unsigned char)255 : (unsigned char)0), ((off[1] != 0) ? (unsigned char)255 : (unsigned char)0), ((off[2] != 0) ? (unsigned char)255 : (unsigned char)0) };
				
			auto pose = marcher.getAbsPos();
			for (uint8_t i = 0; i < DIMENSIONS; ++i)
				curVoxPos[i] = floor(pose[i]);

			keepTracing = !marcher.checkFinished();
	}

	// calculate length in chunks
	ray.length += utils::calculateDist(ray.start, marcher.getAbsPos(), float(sideSteps));

	return ray.color;
}

std::vector<uint8_t> CPURenderer::_rayTraceVoxel(const VoxelMap& map, const VoxelChunk& chunk, const Voxel& vox, Ray& ray, const std::vector<float>& absPose, float voxSide) const
{
	Eigen::Vector3f orig = { ray.start[X], ray.start[Y], ray.start[Z] };
	Eigen::Vector3f dir = { ray.direction[X], ray.direction[Y], ray.direction[Z] };
	dir.normalize();

	Eigen::Vector3f hit;
	Eigen::Vector3f normal;

	if (vox.type == 0)
		ray.length = Cube::rayTrace(orig, dir, hit, normal);
	else if (vox.type == 1)
		ray.length = Sphere::rayTrace(orig, dir, hit, normal);

	normal.normalize();

	float voxDiv = (std::pow(chunk.pyramid.base, chunk.pyramid.power));

	std::vector<float> dists;
	std::vector<float> colorCoeffs = { 0,0,0 };
	auto color = vox.color;

	if (!ray.lightRay && ray.length >= 0)
	{
		for (auto& lightsInChunk : map.getLightsByChunks())
		{
			for (auto& light : lightsInChunk)
			{
				Eigen::Vector3f absRayStart = { absPose[X] + voxSide * hit[X]/voxDiv, absPose[Y] + voxSide * hit[Y]/voxDiv, absPose[Z] + voxSide * hit[Z]/voxDiv };
				Eigen::Vector3f dirToLight = { (light.position[X] - absRayStart[X]), (light.position[Y] - absRayStart[Y]), (light.position[Z] - absRayStart[Z]) };
				float lightDist = sqrt(dirToLight[X] * dirToLight[X] + dirToLight[Y] * dirToLight[Y] + dirToLight[Z] * dirToLight[Z]);
				dirToLight.normalize();

				Ray lightRay(1, { light.position[X], light.position[Y], light.position[Z] }, -dirToLight, 1.0f, true);
				
				_rayTraceMap(map, lightRay);

				float lightStr = 0;
				if (lightRay.length + 0.001f >= lightDist)
				{
					float dotVal = dirToLight.dot(normal);
					lightStr = (dotVal < 0) ? 0 : dotVal;
				}

				for (uint8_t i = 0; i < RGB; ++i)
					colorCoeffs[i] += (light.rgba[i] / 255.0f) * lightStr;
			}
		}

		for (uint8_t i = 0; i < RGB; ++i)
		{
			colorCoeffs[i] = (colorCoeffs[i] > 1.0f) ? 1.0f : (colorCoeffs[i] < 0.0f) ? 0 : colorCoeffs[i];
			color[i] = uint8_t(color[i] * colorCoeffs[i]);
		}
	}

	// dbg normal
	//return { uint8_t(128.0f + normal[X] * 127.0f), uint8_t(128.0f + normal[Y] * 127.0f), uint8_t(128.0f + normal[Z] * 127.0f) };

	// dbg normal + light
	//return { uint8_t((128.0f + normal[X] * 127.0f) * colorCoeffs[X]), uint8_t((128.0f + normal[Y] * 127.0f) * colorCoeffs[Y]), uint8_t((128.0f + normal[Z] * 127.0f) * colorCoeffs[Z]) };

	return color;
}

Voxel CPURenderer::getVoxelData(const VoxelMap& map, const VoxelPyramid& pyram, const std::vector<uint32_t>& pos) const
{
	Voxel vox;

	uint32_t nLeavesBeforeCurrent = 0;
	uint32_t curPwr = 0;
	uint32_t curLayerLen = 1;
	std::vector<uint32_t> leavesOnLayers;
	uint32_t nOffsetBytes = *((uint32_t*)pyram.data.data());
	uint8_t voxSizeInBytes = pyram.data[4];
	uint8_t* ptr = (uint8_t*)pyram.data.data() + sizeof(uint32_t) + sizeof(uint8_t);

	for (uint8_t i = 0; i < pyram.power + 1; ++i)
	{
		leavesOnLayers.push_back(*((uint32_t*)ptr));
		ptr += sizeof(uint32_t);
	}

	uint32_t curPwrLayerPos = 0;

	uint32_t totalWidth = std::pow(pyram.base, pyram.power);
	uint32_t zLayerLen = std::pow(pyram.base, DIMENSIONS - 1);
	uint32_t yRowLen = std::pow(pyram.base, DIMENSIONS - 2);

	uint8_t bytesForThisLayer = VoxelPyramid::getPyramLayerBytesCount(pyram.base, curPwr);

	bool offsetIsFinal = false;
	while (!offsetIsFinal)
	{
		int8_t* offset = (int8_t*)ptr;
		int32_t val;

		if (bytesForThisLayer == 1)
			val = *(offset);
		else if (bytesForThisLayer == 2)
			val = *((int16_t*)offset);
		else
			val = *((int32_t*)offset);

		bool offsetIsFinal = val < 0;
		
		val = offsetIsFinal ? (-val - 1) : val;
		nLeavesBeforeCurrent += (offsetIsFinal ? val : leavesOnLayers[curPwr]);

		if (offsetIsFinal)
			break;

		ptr += bytesForThisLayer * uint32_t(curLayerLen - curPwrLayerPos); // skipping to the end of current layer

		uint32_t curSide = totalWidth / std::pow(pyram.base, curPwr);
		uint32_t sidePart = curSide / pyram.base;
		curPwrLayerPos = ((pos[Z] % curSide) / sidePart) * zLayerLen + ((pos[Y] % curSide) / sidePart) * yRowLen + ((pos[X] % curSide) / sidePart);

		curLayerLen -= leavesOnLayers[curPwr];
		curLayerLen *= std::pow(pyram.base, DIMENSIONS);

		curPwr++;
		bytesForThisLayer = VoxelPyramid::getPyramLayerBytesCount(pyram.base, curPwr);
		ptr += bytesForThisLayer * uint32_t(val * std::pow(pyram.base, DIMENSIONS) + curPwrLayerPos); // skipping to the value of interest
	}

	ptr = (uint8_t*)pyram.data.data() + sizeof(uint32_t) + sizeof(uint8_t) +
		leavesOnLayers.size() * sizeof(uint32_t) + nOffsetBytes;

	ptr += voxSizeInBytes * nLeavesBeforeCurrent;

	auto voxData = map.getType().unformatVoxelData(ptr);
	vox.type = std::get<0>(voxData);
	vox.color = std::get<1>(voxData);
	vox.neighs = std::get<2>(voxData);
	vox.power = curPwr;

	return vox;
}
