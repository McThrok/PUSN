#pragma once
class GuiData
{
public:
	bool paused = true;
	int gridX = 100;
	int gridY = 100;
	float toolRadius = 6;

	bool flat = true;

	float speed = 0.5;
	float materialDepth = 4;
	float toolDepth = 4;
	bool wireframe = true;

	bool toolDepthViolated = false;
	bool materialDepthViolated = false;
};

