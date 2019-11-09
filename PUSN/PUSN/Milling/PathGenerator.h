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
#include <stdio.h>
#include <direct.h>

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
#include <memory>

using namespace std;
using namespace DirectX;

class PathGenerator
{
public:
	MillingMaterial* material;
	vector< shared_ptr<BezierSurfaceC0>> model;

	void InitMaterial();
	void LoadElephant();
};

