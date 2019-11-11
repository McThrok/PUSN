#pragma once
#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
#include "..\\Graphics\\Texture.h"
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

#include "..\\Graphics\\AdapterReader.h"
#include "..\\Graphics\\Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "..\\Graphics\\Camera3D.h"
#include "..\\Graphics\\RenderableGameObject.h"
#include "..\\Graphics\\Light.h"
#include <memory>
#include "MillingMaterial.h"
#include "Bezier/BezierSurface.h"
#include "Bezier/IntersectionCurve.h"
#include "../StringHelper.h"

using namespace std;
using namespace DirectX;

class PathGenerator
{
public:
	int resX, resY;
	XMFLOAT3 size;
	XMMATRIX modelTransform;

	vector<vector<float>> heightMap;

	PathGenerator(int _resX, int _resY, XMFLOAT3 _size);
	vector<shared_ptr<BezierSurfaceC0>> model;
	vector<BezierSurfaceC0*> GetModel();


	void SavePath(vector<XMFLOAT3> moves, string filePath);

	void LoadElephant();
	void GenerateHeightMap();
	vector<XMFLOAT3> GenerateFirstPath();
	void GeneratePaths();
};

