#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class GuiData
{
public:
	bool paused = true;

	int gridX = 100;
	int gridY = 100;
	DirectX::XMFLOAT3 size = DirectX::XMFLOAT3(150, 150, 50);

	float toolRadius = 6;
	bool flat = true;

	bool wireframe = true;
	bool flatShading = false;
	bool showPath = false;
};

