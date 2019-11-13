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
#include <SimpleMath.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

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
	Vector3 currentPosition;
	Vector3 safePosition;

	vector<Vector3> moves;

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;

	Vector3 Normalize(Vector3 v);
	Vector3 Move();
	void Cut(Vector3 dir, MillingMaterial * material);
	Vector3 CalculateNormal(const Vector3 &left, const Vector3 &right, const Vector3 &top, const Vector3 &down);
};


