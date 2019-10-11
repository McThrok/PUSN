#pragma once
#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
#include "..\\Graphics\\Texture.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>

#include "..\\Graphics\\AdapterReader.h"
#include "..\\Graphics\\Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "..\\Graphics\\Camera3D.h"
#include "..\\Graphics\\RenderableGameObject.h"
#include "..\\Graphics\\Light.h"
#include "..\\Graphics\\Camera2D.h"
#include "..\\Graphics\\Sprite.h"
#include <memory>

class MillingMaterial
{
public:
	MillingMaterial(ID3D11Device * device, ID3D11DeviceContext * deviceContext);
	MillingMaterial(const MillingMaterial & milligMaterial);

	void Initialize(XMFLOAT3 size, int _gridX, int _gridZ);
	void Draw();
	void UpdateVertexBuffer();
	void Randomize();
	void Reset();
	Vertex3D& GetVert(int x, int z);

	int gridX, gridZ;
	XMFLOAT3 size;

private:

	std::vector<Vertex3D> vertices;
	VertexBuffer<Vertex3D> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext * deviceContext;
	ID3D11Device * device;
};

