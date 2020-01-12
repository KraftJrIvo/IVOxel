#include "CPURenderer.h"

CPURenderer::CPURenderer()
{
}

void CPURenderer::render(const VoxelMap& map, Camera& cam)
{
	while (true)
	{
		cv::Mat result(cam.resolution[1], cam.resolution[0], CV_8UC3, cv::Scalar(0, 0, 0));

		for (uint32_t i = 0; i < cam.resolution[0]; ++i)
			for (uint32_t j = 0; j < cam.resolution[1]; ++j)
			{
				//std::cout << i << " " << j << std::endl;
				auto pixel = _renderPixel(map, cam, i, j);
				result.at<cv::Vec3b>(j, i) = { pixel[0], pixel[1], pixel[2] };
			}

		cv::imshow("result", result);
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

std::vector<uint8_t> CPURenderer::_renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y)
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

std::vector<uint8_t> CPURenderer::_rayTraceMap(const VoxelMap& map, Ray& ray)
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
	bool notFinish = true;// map.checkIfChunkIsPossible(curPos, ray.direction);

	while (notFinish)
	{
		auto* chunk = map.getChunk(curChunkPos);
		if (chunk)
		{
			Ray entryRay(ray.bouncesLeft, marcher.getCurEntryPoint(), ray.direction, ray.strength);
			resultColor = _rayTraceChunk(map, *chunk, entryRay);
			ray.bouncesLeft = entryRay.bouncesLeft;
		}

		notFinish = ray.bouncesLeft > 0 && !marcher.checkFinished();

		auto off = marcher.marchAndGetNextDir(map.getMinMaxChunks());

		for (uint8_t i = 0; i < DIMENSIONS; ++i)
			curChunkPos[i] += off[i];
	}

	return resultColor;
}

std::vector<uint8_t> CPURenderer::_rayTraceChunk(const VoxelMap& map, const VoxelChunk& chunk, Ray& ray)
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
			//bool solid = (25 * curVoxPos[Z] + 5 * curVoxPos[Y] + curVoxPos[X]) % 2 == 0;
			//if (solid)
			//{
			//	vox.type = 0;
			//	vox.color = { 200, 200, 200 };
			//	break;
			//}

			vox = getVoxelData(map, chunk.pyramid, curVoxPos);
			if (vox.type == -1)
				stepsToTake = 1; //sideSteps / std::pow(chunk.pyramid.base, vox.power);
			else
				break;


			// TODO optimize stepsToTake
			for (uint8_t s = 0; s < stepsToTake; ++s)
			{
				auto off = marcher.marchAndGetNextDir(minMaxs);
				for (uint8_t i = 0; i < DIMENSIONS; ++i)
					curVoxPos[i] += off[i];
			}
			keepTracing = !marcher.checkFinished();
	}

	if (vox.type != -1)
	{
		Ray voxRay = ray;
		voxRay.start = marcher.getCurEntryPoint();
		// TODO ray.bounce();
		ray.bouncesLeft--;
		return _rayTraceVoxel(vox, voxRay);
	}
	return ray.color;
}

std::vector<uint8_t> CPURenderer::_rayTraceVoxel(const Voxel& vox, const Ray& ray)
{
	//TODO

	return vox.color;
}

std::vector<uint8_t> CPURenderer::mixRayColor(const std::vector<uint8_t>& color, const Ray& ray)
{
	return std::vector<uint8_t>();
}

Voxel CPURenderer::getVoxelData(const VoxelMap& map, const VoxelPyramid& pyram, const std::vector<uint32_t>& pos)
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	std::vector<uint16_t> bytesForLayers;
	for (uint32_t i = 0; i < pyram.power + 1; ++i)
	{
		uint32_t vol = std::pow(std::pow(pyram.base, i), DIMENSIONS);
		uint16_t bytesForThis = uint16_t(std::ceil(std::log2(vol) / std::log2(pyram.base) / 8));
		bytesForThis = (bytesForThis == 1 || bytesForThis == 2) ? bytesForThis : (bytesForThis == 0 ? 1 : 4);
		bytesForLayers.push_back(bytesForThis);
	}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	Voxel vox;

	uint32_t nLeavesBeforeCurrent = 0;
	uint32_t curPwr = 0;
	//uint32_t curOff = 2;
	uint32_t curLayerLen = 1;
	//std::vector<uint32_t> offsets;
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

	bool offsetIsFinal = false;
	while (!offsetIsFinal)
	{
		int8_t* offset = (int8_t*)ptr;
		int32_t val;
		if (bytesForLayers[curPwr] == 1)
			val = *(offset);
		else if (bytesForLayers[curPwr] == 2)
			val = *((int16_t*)offset);
		else
			val = *((int32_t*)offset);

		bool offsetIsFinal = val < 0;
		
		val = offsetIsFinal ? (-val - 1) : val;
		//offsets.push_back(val);
		nLeavesBeforeCurrent += (offsetIsFinal ? val : leavesOnLayers[curPwr]);

		if (offsetIsFinal)
			break;

		ptr += bytesForLayers[curPwr] * uint32_t(curLayerLen - curPwrLayerPos); // skipping to the end of current layer

		uint32_t curSide = totalWidth / std::pow(pyram.base, curPwr);
		uint32_t sidePart = curSide / pyram.base;
		curPwrLayerPos = ((pos[Z] % curSide) / sidePart) * zLayerLen + ((pos[Y] % curSide) / sidePart) * yRowLen + ((pos[X] % curSide) / sidePart);

		uint32_t fff = val * std::pow(pyram.base, DIMENSIONS);
		ptr += fff; // skipping to the end of current layer
		ptr += bytesForLayers[curPwr] * curPwrLayerPos;  // skipping to the start of current offset

		curLayerLen -= leavesOnLayers[curPwr];
		curLayerLen *= std::pow(pyram.base, DIMENSIONS);
		curPwr++;
	}

	ptr = (uint8_t*)pyram.data.data() + 5 + leavesOnLayers.size() * sizeof(uint32_t) + nOffsetBytes;

	ptr += voxSizeInBytes * nLeavesBeforeCurrent;

	//std::cout << pos[X] << pos[Y] << pos[Z] << std::endl;

	auto voxData = map.getType().unformatVoxelData(ptr);
	vox.type = std::get<0>(voxData);
	vox.color = std::get<1>(voxData);
	vox.neighs = std::get<2>(voxData);
	vox.power = curPwr;

	return vox;
}
