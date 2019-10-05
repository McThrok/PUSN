#pragma once
#include "RenderWindow.h"
#include "Keyboard/KeyboardClass.h"
#include "Mouse/MouseClass.h"
#include "Graphics/Graphics.h"
#include "ErrorLogger.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool InitializeBASE(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	HWND GetHWND() const;
	bool ProcessMessageseBASE();
	~WindowContainer();

protected:
	KeyboardClass keyboard;
	MouseClass mouse;
	Graphics gfx;
private:
	void RegisterWindowClass();
	HWND handle = NULL; //Handle to this window
	HINSTANCE hInstance = NULL; //Handle to application instance
	std::string window_title = "";
	std::wstring window_title_wide = L""; //Wide string representation of window title
	std::string window_class = "";
	std::wstring window_class_wide = L""; //Wide string representation of window class name
	int width = 0;
	int height = 0;
};