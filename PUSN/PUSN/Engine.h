#pragma once
#include "WindowContainer.h"
#include "Timer.h"
#include <memory>

class Engine : public WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void Update();
	void RenderFrame();
private:
	Timer timer;
};