#pragma once
#include <DirectXMath.h>

class GuiData
{
public:
	bool paused = true;
	int gridX = 100;
	int gridY = 100;
	DirectX::XMFLOAT3 size = DirectX::XMFLOAT3(100, 50, 150);
	float toolRadius = 6;

	bool flat = true;

	float speed = 0.5;
	float materialDepth = 50;
	float toolDepth = 50;
	bool wireframe = true;

	bool toolDepthViolated = false;
	bool materialDepthViolated = false;
};

