#include "CPURenderer.h"

#include <algorithm>

#include <opencv2/videoio.hpp>

#include "Cube.h"
#include "Sphere.h"

#include <omp.h>

Window window(512, 512, L"test");

void CPURenderer::_drawImage(cv::Mat img)
{
	HWND wnd = window.getHWND();

	int width = img.cols;
	int height = img.rows;
	int bpp = 3;
	const unsigned char* buffer = img.data;

    RECT rect;
    GetWindowRect(wnd, &rect);
    auto dc = GetDC(wnd);

	BITMAPINFO bmpinfo;

	memset(&bmpinfo, 0, sizeof(bmpinfo));
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biBitCount = 24;
	bmpinfo.bmiHeader.biClrImportant = 0;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biWidth = width;
	bmpinfo.bmiHeader.biHeight = -height;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biSizeImage = 0;
	bmpinfo.bmiHeader.biXPelsPerMeter = 100;
	bmpinfo.bmiHeader.biYPelsPerMeter = 100;

	::SetStretchBltMode(dc, COLORONCOLOR);
	::StretchDIBits(dc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, 0, width, height, buffer, &bmpinfo, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(wnd, dc);
}

void CPURenderer::startRender()
{
	auto cam = _gs.getCam();
	auto map = _gs.getMap();

	while (_runOnce())
	{
		cv::Mat result(cam.res[1], cam.res[0], CV_8UC3, cv::Scalar(0, 0, 0));

		std::vector<int32_t> pos = { (int)cam.pos.x, (int)cam.pos.y, (int)cam.pos.z };
		if (map->checkLoadNeeded(pos))
			_raytracer.setMapData(map->getChunksDataAt(pos, _chunkLoadRadius, _alignToFourBytes));
		_raytracer.setLightData({ 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.33f }, map->getLightDataAt(pos, _chunkLoadRadius));

		#pragma omp parallel
		{
			uint8_t nThreads = omp_get_num_threads();
			uint8_t threadId = omp_get_thread_num();

			for (uint32_t i = threadId; i < cam.res[0]; i += nThreads)
				for (uint32_t j = 0; j < cam.res[1]; ++j)
				{
					auto pixel = _renderPixel(*map, cam, { i, j });
					result.at<cv::Vec3b>(j, i) = { pixel[2], pixel[1], pixel[0] };
				}
			
			#pragma omp barrier
		}

		_drawImage(result);
	}
}

void CPURenderer::stop()
{
	window.Close();
}

bool CPURenderer::_runOnce()
{
	return window.Update();
}

std::vector<uint8_t> CPURenderer::_renderPixel(const VoxelMap& map, const Camera& cam, glm::vec2 xy) const
{
	using namespace glm;

	vec2 coeffs = (xy - cam.res / 2.0f) / cam.res.y;

	vec3 coords = vec3(coeffs.y, -coeffs.x, -1.0);

	vec3 start = cam.pos;
	vec3 dir = mat3(cam.mvp) * coords;

	int bounces = 1;
	float len = 0;

	vec3 normal, color;
	auto hit = _raytracer.raytraceMap(start, dir, normal, color);

	std::vector<uint8_t> result = { uint8_t(255 * color.r), uint8_t(255 * color.g), uint8_t(255 * color.b) };

	return result;
}