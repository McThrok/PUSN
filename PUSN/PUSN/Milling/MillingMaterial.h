#pragma once
#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>

#include "..\\Graphics\\AdapterReader.h"
#include "..\\Graphics\\Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "..\\Graphics\\Camera3D.h"
#include "Bezier/BezierSurface.h"
#include <memory>
#include <SimpleMath.h>
#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class MillingMaterial
{
public:
	MillingMaterial(ID3D11Device * device, ID3D11DeviceContext * deviceContext);
	MillingMaterial(const MillingMaterial & milligMaterial);

	void Initialize(Vector3 size, int _gridX, int _gridY);
	void Draw();
	void UpdateVertexBuffer();
	void Reset();
	Vertex3D& GetVert(int x, int y);
	void GetIndicesOfArea(Vector3 position, float range, int & left, int & right, int & top, int & down);

	int gridX, gridY;
	Vector3 size;

private:
	std::vector<Vertex3D> vertices;
	VertexBuffer<Vertex3D> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext * deviceContext;
	ID3D11Device * device;
};

