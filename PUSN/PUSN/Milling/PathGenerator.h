#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <direct.h>
#include <iomanip>
#include <SimpleMath.h>
#include <memory>

#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
#include "Model.h"
#include "..\\Graphics\\Shaders.h"
#include "MillingMaterial.h"
#include "Bezier/BezierSurface.h"
#include "Bezier/IntersectionCurve.h"
#include "../StringHelper.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class PathGenerator
{
public:

	Model model;
	MillingMaterial* material;
	vector<vector<float>> heightMap;
	float minZ;
	float safeZ;

	PathGenerator(MillingMaterial* _material);

	void SavePath(vector<Vector3> moves, string filePath);

	void GenerateFirstPath();
	void GenerateSecondPath();
	void GenerateHeightMap();
	float GetZ(float cpx, float cpy, bool flat, float toolRadius);

	void EnsureInit();
	vector<Vector3> GenerateFirstPathLayer(float minZ);
	vector<Vector3> GenerateFlatEnvelope();
	BezierSurfaceC0 GetPlane();
	vector<Vector3> GenerateUnrestrictedPath(BezierSurfaceC0* surface, Vector3 startingPoint);
	vector<Vector3> GenerateUnrestrictedCylinderPath(BezierSurfaceC0* surface, bool top);
	bool SegmentsIntersect(const Vector2& A, const Vector2& B, const Vector2& C, const Vector2& D, Vector2& out);
	void TrimStart(vector<Vector3>& trimmer, vector<Vector3>& path);
	void TrimEnd(vector<Vector3>& path, vector<Vector3>& trimmer);
	void RemoveSelfIntersections(vector<Vector3>& path);

	vector<Vector3> GenerateFlatLayer();

};

