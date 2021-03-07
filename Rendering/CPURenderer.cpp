#include "CPURenderer.h"

#include <algorithm>

#include <opencv2/videoio.hpp>

#include "Cube.h"
#include "Sphere.h"

#include <omp.h>

CPURenderer::CPURenderer()
{
}

void CPURenderer::render(const VoxelMap& map, Camera& cam)
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
	/*std::vector<uint8_t> rgb = {0,0,0};

	float halfRes = cam.resolution[0] / 2;
	float halfFovRad = (cam.fov / 180.0f) * 3.14159f / 2.0f;

	float coeffX = (float(x) - halfRes) / halfRes;
	float coeffY = -(float(y) - cam.resolution[1] / 2) / halfRes;

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

	return _rayTraceMap(map, ray);*/

	//vec2 coeffs = (gl_FragCoord.xy - view.resolution.xy / 2.0) / view.resolution.y;

	//vec3 coords = vec3(coeffs.y, -coeffs.x, -1.0);

	//vec3 start = view.pos;
	//vec3 dir = mat3(view.mvp) * coords;

	//int bounces = 1;
	//float len = 0;

	//outColor = vec4(raytraceMap(start, dir, bounces, len), 1.0);

	//for (uint i = 0; i < light.nLights; ++i)
	//{
	//	float lightLen = length(start - light.coords[i].xyz);
	//	if (lightLen < len)
	//	{
	//		float lightDist = getPtLineDist(start, dir, light.coords[i].xyz);
	//		float g = (0.1 - lightDist) * 10.0;
	//		float coeff = g < 0 ? 0 : g;
	//		vec3 c = coeff * light.colors[i].xyz;
	//		outColor.xyz += c;
	//	}
	//}
}