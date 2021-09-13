#include "Window.h"

#include <algorithm>

Window::Window(uint32_t size_x, uint32_t size_y, std::wstring name, float renderScale)
{
	this->renderScale = renderScale;
	_surface_size_x = size_x;
	_surface_size_y = size_y;
	_window_name = name;

	_InitOSWindow();
}

Window::~Window()
{
	_DeInitOSWindow();
}


void Window::Close()
{
	_window_should_run = false;
}

bool Window::Update()
{
	_UpdateOSWindow();
	return _window_should_run;
}

void Window::setSurfaceSize(uint32_t w, uint32_t h)
{
	_surface_size_x = w;
	_surface_size_y = h;
	justResized = true;
}

std::pair<uint32_t, uint32_t> Window::getSize()
{
	return {_surface_size_x, _surface_size_y };
}

std::pair<uint32_t, uint32_t> Window::getSizeScaled()
{
	return { _surface_size_x / renderScale, _surface_size_y / renderScale };
}

bool Window::wasResized()
{
	if (justResized && !lmbDown)
	{
		justResized = false;
		return true;
	}
	return false;
}

float Window::getRenderScale()
{
	return renderScale;
}

glm::ivec4 Window::getRenderArea()
{
	auto wSz = getSize();
	return glm::ivec4(0, 0, (uint32_t)(wSz.first / renderScale), (uint32_t)(wSz.second / renderScale));
}

glm::ivec4 Window::getRenderAreaScaled()
{
	auto wSz = getSize();
	return glm::ivec4(0, 0, wSz.first, wSz.second);
}

const HINSTANCE& Window::getHInstance()
{
	return _win32_instance;
}

const HWND& Window::getHWND()
{
	return _win32_window;
}

// Microsoft Windows specific versions of window functions
LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_SIZE:
		window->setSurfaceSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		window->mousePos = { LOWORD(lParam), HIWORD(lParam) };
		window->lmbDown = true;
		break;
	case WM_LBUTTONUP:
		window->lmbDown = false;
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 81:
			window->upPressed = true;
			break;
		case 69:
			window->downPressed = true;
			break;
		case 65:
			window->leftPressed = true;
			break;
		case 68:
			window->rightPressed = true;
			break;
		case 83:
			window->backwardPressed = true;
			break;
		case 87:
			window->forwardPressed = true;
			break;
		case VK_ADD:
			window->renderScale *= 2.0f;
			window->justResized = true;
			break;
		case VK_SUBTRACT:
			window->renderScale /= 2.0f;
			if (window->renderScale < 1.0f) window->renderScale = 1.0f;
			else window->justResized = true;
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		if (wParam == VK_F11)
			window->fullScreenSwitch();
		switch (wParam)
		{
		case 81:
			window->upPressed = false;
			break;
		case 69:
			window->downPressed = false;
			break;
		case 65:
			window->leftPressed = false;
			break;
		case 68:
			window->rightPressed = false;
			break;
		case 83:
			window->backwardPressed = false;
			break;
		case 87:
			window->forwardPressed = false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

uint64_t	Window::_win32_class_id_counter = 0;

void Window::_InitOSWindow()
{
	WNDCLASSEX win_class{};
	assert(_surface_size_x > 0);
	assert(_surface_size_y > 0);

	_win32_instance = GetModuleHandle(nullptr);
	_win32_class_name = _window_name;// +"_" + std::to_string(_win32_class_id_counter);
	_win32_class_id_counter++;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = _win32_instance; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = (LPCWSTR)_win32_class_name.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	_styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;

	// Create window with the registered class:
	RECT wr = { 0, 0, LONG(_surface_size_x), LONG(_surface_size_y) };
	AdjustWindowRectEx(&wr, _style, FALSE, _styleEx);
	_win32_window = CreateWindowEx(0,
		(WCHAR*)_win32_class_name.c_str(),		// class name
		(WCHAR*)_window_name.c_str(),			// app name
		_style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		_win32_instance,				// hInstance
		NULL);							// no extra parameters
	if (!_win32_window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(_win32_window, GWLP_USERDATA, (LONG_PTR)this);

	auto hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&pDI, NULL);

	hr = pDI->CreateDevice(GUID_SysMouse, &pMouse, NULL);

	hr = pMouse->SetDataFormat(&c_dfDIMouse2);

	hr = pMouse->SetCooperativeLevel(_win32_window, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	/*if (!bImmediate)
	{
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 16; // Arbitrary buffer size

		if (FAILED(hr = pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			return hr;
	}*/

	pMouse->Acquire();

	ShowWindow(_win32_window, SW_SHOW);
	SetForegroundWindow(_win32_window);
	SetFocus(_win32_window);
}

void Window::_DeInitOSWindow()
{
	DestroyWindow(_win32_window);
	UnregisterClass((WCHAR*)_win32_class_name.c_str(), _win32_instance);
}

void Window::_UpdateOSWindow()
{
	MSG msg;
	if (PeekMessage(&msg, _win32_window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::fullScreenSwitch()
{
	HWND HWNDWindow = getHWND();

	if (_windowed)
	{
		_windowed = FALSE;
		GetWindowPlacement(HWNDWindow, &_wpc);
		if (_style == 0)
			_style = GetWindowLong(HWNDWindow, GWL_STYLE);
		if (_styleEx == 0)
			_styleEx = GetWindowLong(HWNDWindow, GWL_EXSTYLE);

		LONG NewHWNDStyle = _style;
		NewHWNDStyle &= ~WS_BORDER;
		NewHWNDStyle &= ~WS_DLGFRAME;
		NewHWNDStyle &= ~WS_THICKFRAME;

		LONG NewHWNDStyleEx = _styleEx;
		NewHWNDStyleEx &= ~WS_EX_WINDOWEDGE;

		SetWindowLong(HWNDWindow, GWL_STYLE, NewHWNDStyle | WS_POPUP);
		SetWindowLong(HWNDWindow, GWL_EXSTYLE, NewHWNDStyleEx | WS_EX_TOPMOST);
		ShowWindow(HWNDWindow, SW_SHOWMAXIMIZED);
	}
	else
	{
		_windowed = TRUE;
		SetWindowLong(HWNDWindow, GWL_STYLE, _style);
		SetWindowLong(HWNDWindow, GWL_EXSTYLE, _styleEx);
		ShowWindow(HWNDWindow, SW_SHOWNORMAL);
		SetWindowPlacement(HWNDWindow, &_wpc);
	}
}

std::vector<float> Window::getCurDeltaRot()
{
	DIMOUSESTATE2 dims2;
	ZeroMemory(&dims2, sizeof(dims2));

	auto hr = pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &dims2);

	if (!lmbDown)
		return { 0,0 };

	if (FAILED(hr))
	{
		hr = pMouse->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = pMouse->Acquire();

		return { 0,0 };
	}

	return { -dims2.lY / renderScale, dims2.lX / renderScale };
}

std::vector<float> Window::getCurDeltaTrans()
{
	std::vector<float> delta = { 0,0,0 };

	delta[0] += rightPressed ? -1.0f : 0.0f;
	delta[0] += leftPressed ? 1.0f : 0.0f;
	delta[1] += downPressed ? -1.0f : 0.0f;
	delta[1] += upPressed ? 1.0f : 0.0f;
	delta[2] += backwardPressed ? -1.0f : 0.0f;
	delta[2] += forwardPressed ? 1.0f : 0.0f;

	return delta;
}