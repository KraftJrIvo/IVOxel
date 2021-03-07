#pragma once

#include <string>
#include <vector>

#include <windows.h>

#include <glm/glm.hpp>

class Window
{
public:
	Window(uint32_t size_x, uint32_t size_y, std::wstring name, float renderScale = 1.0f);
	~Window();

	void Close();
	bool Update();
	void setSurfaceSize(uint32_t w, uint32_t h);
	std::pair<uint32_t, uint32_t> getSize();
	std::pair<uint32_t, uint32_t> getSizeScaled();
	bool wasResized();
	float getRenderScale();
	glm::ivec4 getRenderArea();
	glm::ivec4 getRenderAreaScaled();

	const HINSTANCE& getHInstance();
	const HWND& getHWND();

	void handleMouseMove(int32_t x, int32_t y);
	std::vector<float> getCurDeltaRot();
	std::vector<float> getCurDeltaTrans();

	void fullScreenSwitch();

	std::vector<int32_t> mousePos = {0,0};
	bool lmbDown         = false;
	bool upPressed       = false;
	bool downPressed     = false;
	bool leftPressed     = false;
	bool rightPressed    = false;
	bool forwardPressed  = false;
	bool backwardPressed = false;

	float renderScale = 1.0;
	bool justResized = false;

private:
	void _InitOSWindow();
	void _DeInitOSWindow();
	void _UpdateOSWindow();


	uint32_t _surface_size_x = 512;
	uint32_t _surface_size_y = 512;
	std::wstring _window_name;

	bool _window_should_run = true;

	HINSTANCE		 _win32_instance = NULL;
	HWND			 _win32_window = NULL;
	std::wstring	 _win32_class_name;
	static uint64_t	 _win32_class_id_counter;

	std::vector<float> _deltaRot = { 0,0 };

	bool			_windowed = true;
	WINDOWPLACEMENT _wpc;
	DWORD		    _style = 0;
	DWORD		    _styleEx = 0;

};