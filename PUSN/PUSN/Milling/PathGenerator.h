#pragma once
#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
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
	MillingMaterial* material;

	vector<vector<float>> heightMap;

	PathGenerator(MillingMaterial* _material);
	vector<shared_ptr<BezierSurfaceC0>> model;
	vector<BezierSurfaceC0*> GetModel();


	void SavePath(vector<Vector3> moves, string filePath);

	void LoadElephant();
	void GeneratePaths();

	void GenerateHeightMap();
	vector<Vector3> GenerateFirstPathLayer(float minZ);
	float GetZ(float cpx, float cpy, bool flat);

	vector<Vector3> GenerateFlatLayer(float minZ);
	vector<Vector3> GenerateFlatEnvelope(float minZ);


};

