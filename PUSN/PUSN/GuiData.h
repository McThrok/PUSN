#pragma once
#include <DirectXMath.h>

class GuiData
{
public:
	bool paused = true;

	int gridX = 100;
	int gridZ = 100;
	DirectX::XMFLOAT3 size = DirectX::XMFLOAT3(100, 50, 150);

	float toolRadius = 6;
	bool flat = true;

	bool wireframe = true;
	bool flatShading = false;
};

