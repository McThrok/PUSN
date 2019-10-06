#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>

#include "AdapterReader.h"
#include "Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Camera3D.h"
#include "..\\Timer.h"
#include "ImGUI\\imgui.h"
#include "ImGUI\\imgui_impl_win32.h"
#include "ImGUI\\imgui_impl_dx11.h"
#include "RenderableGameObject.h"
#include "Light.h"
#include "Camera2D.h"
#include "Sprite.h"
#include <memory>

class MillingMaterial
{
public:
	MillingMaterial(ID3D11Device * device, ID3D11DeviceContext * deviceContext);
	MillingMaterial(const MillingMaterial & milligMaterial);

	void Initialize(int gridX, int gridZ);
	void Draw();
	void UpdateVertexBuffer();
	void Randomize();
	Vertex3D& Get(int x, int z);

private:
	int gridX, gridZ;

	std::vector<Vertex3D> vertices;
	VertexBuffer<Vertex3D> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext * deviceContext;
	ID3D11Device * device;
};

