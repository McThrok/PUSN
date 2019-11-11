#pragma once
#include <DirectXMath.h>

class GuiData
{
public:
	bool paused = true;

	int gridX = 300;
	int gridY = 300;
	DirectX::XMFLOAT3 size = DirectX::XMFLOAT3(150, 150, 50);

	float toolRadius = 6;
	bool flat = true;

	bool wireframe = true;
	bool flatShading = false;
	bool showPath = false;
};

