#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <math.h>
#include <d3d11.h>

#include "MillingMaterial.h"
#include "..\\Graphics\\Mesh.h"

using namespace DirectX;
using namespace std;

class MillingMachine {
public:
	MillingMachine(ID3D11Device * _device, ID3D11DeviceContext * _deviceContext);

	void LoadDataFromFile(string filePath);
	void SetMillingCutterMesh(float radius, bool flat);
	void SetPathMesh();
	void Reset();
	void Update(float dt, MillingMaterial * material);

	shared_ptr<Mesh> millingCutterMesh;
	shared_ptr<Mesh> pathMesh;

	bool flatCut;
	float cutRadius;
	float speed;
	float stepSize;
	bool finished;

	float materialDepth;
	float toolDepth;

	bool toolDepthViolated;
	bool materialDepthViolated;
	bool millingViolated;

private:
	float restTime;
	int currentMove;
	XMFLOAT3 currentPosition;
	XMFLOAT3 safePosition;

	vector<XMFLOAT3> moves;

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;

	XMFLOAT3 Normalize(XMFLOAT3 v);
	XMFLOAT3 Move();
	void Cut(XMFLOAT3 dir, MillingMaterial * material);
	XMFLOAT3 CalculateNormal(const XMFLOAT3 &left, const XMFLOAT3 &right, const XMFLOAT3 &top, const XMFLOAT3 &down);
};


