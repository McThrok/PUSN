//Tutorial 64 Solution 2019-03-09
#include "Engine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to call CoInitialize.");
		return -1;
	}

	Engine engine;
	//if (engine.Initialize(hInstance, "Title", "MyWindowClass", 1800, 900))
	if (engine.Initialize(hInstance, "Title", "MyWindowClass", 1400, 700))
	{
		while (engine.ProcessMessage() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}
	}
	return 0;
}