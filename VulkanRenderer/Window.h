#pragma once

#include "VulkanTypes.h"
#include <string>

class Window
{
public:
	Window(uint32_t size_x, uint32_t size_y, std::wstring name);
	~Window();

	void Close();
	bool Update();
	void setSurfaceSize(uint32_t w, uint32_t h);
	std::pair<uint32_t, uint32_t> getSize();
	bool wasResized();
	VkRect2D getRenderArea();

	const HINSTANCE& getHInstance();
	const HWND& getHWND();

	void handleMouseMove(int32_t x, int32_t y);
	std::vector<float> getCurDeltaRot();

	void fullScreenSwitch();

	std::vector<int32_t>				mousePos = {0,0};
	bool								lmbDown = false;

private:
	void								_InitOSWindow();
	void								_DeInitOSWindow();
	void								_UpdateOSWindow();

	bool _wasResized =					false;

	uint32_t							_surface_size_x = 512;
	uint32_t							_surface_size_y = 512;
	std::wstring							_window_name;

	bool								_window_should_run = true;

	HINSTANCE							_win32_instance = NULL;
	HWND								_win32_window = NULL;
	std::wstring							_win32_class_name;
	static uint64_t						_win32_class_id_counter;

	std::vector<float>					_deltaRot = { 0,0 };
	bool								_windowed = true;
	WINDOWPLACEMENT						_wpc;
	DWORD								_style = 0;
	DWORD								_styleEx = 0;

};