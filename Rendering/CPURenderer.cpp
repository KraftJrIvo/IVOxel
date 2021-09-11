#include "CPURenderer.h"

#include <algorithm>

#include <omp.h>

void CPURenderer::_drawImage(cv::Mat img)
{
	HWND wnd = _window.getHWND();

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
	auto& cam = _gs.getCam();
	auto& map = _gs.getMap();

	static std::thread t([&]() {_gs.startUpdateLoop(&_raytracer, 1); });

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	while (_runOnce())
	{
		cv::Mat result(cam.res[1], cam.res[0], CV_8UC3, cv::Scalar(0, 0, 0));

		#pragma omp parallel
		{
			uint8_t nThreads = omp_get_num_threads();
			uint8_t threadId = omp_get_thread_num();

			for (uint32_t i = threadId; i < cam.res[0]; i += nThreads)
				for (uint32_t j = 0; j < cam.res[1]; ++j)
				{
					auto pixel = _renderPixel({ i, j });
					result.at<cv::Vec3b>(j, i) = { pixel[2], pixel[1], pixel[0] };
				}
			
			#pragma omp barrier
		}

		_gs.update(EVERY_FRAME);
		_gs.upload(EVERY_FRAME, &_raytracer);

		_drawImage(result);
	}
}

void CPURenderer::stop()
{
	_window.Close();
}

bool CPURenderer::_runOnce()
{
	return _window.Update();
}

std::vector<uint8_t> CPURenderer::_renderPixel(glm::vec2 xy) const
{
	glm::vec3 normal, color;
	auto hit = _raytracer.raytracePixel(xy, normal, color);

	return { uint8_t(255 * color.r), uint8_t(255 * color.g), uint8_t(255 * color.b) };
}