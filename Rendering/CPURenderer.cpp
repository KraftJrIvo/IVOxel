#include "CPURenderer.h"

CPURenderer::CPURenderer()
{
}

void CPURenderer::render(const VoxelMap& map, const Camera& cam)
{
	cv::Mat result(cam.resolution[1], cam.resolution[0], CV_8UC3, cv::Scalar(0,0,0));

	for (uint32_t i = 0; i < cam.resolution[0]; ++i)
		for (uint32_t j = 0; j < cam.resolution[1]; ++j)
		{
			auto pixel = _renderPixel(map, cam, i, j);
			result.at<cv::Vec3b>(j, i) = { pixel[0], pixel[1], pixel[2] };
		}

	cv::imshow("result", result);
	cv::waitKey(0);
}

std::vector<uint8_t> CPURenderer::_renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y)
{
	std::vector<uint8_t> rgb = {0,0,0};

	Eigen::Quaterniond quat = 
		Eigen::AngleAxisd(cam.rotation[0], Eigen::Vector3d::UnitX())
		* Eigen::AngleAxisd(cam.rotation[1], Eigen::Vector3d::UnitY())
		* Eigen::AngleAxisd(cam.rotation[2], Eigen::Vector3d::UnitZ());
	quat.normalize();
	Eigen::Vector3d viewVec(0,0,1.0);
	viewVec = quat.matrix() * viewVec;
	
	return rgb;
}