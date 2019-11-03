#include "MillingMaterial.h"



MillingMaterial::MillingMaterial(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{
	this->deviceContext = deviceContext;
	this->device = device;
}

MillingMaterial::MillingMaterial(const MillingMaterial & millingMaterial)
{
	this->deviceContext = millingMaterial.deviceContext;
	this->device = millingMaterial.device;
	this->indexbuffer = millingMaterial.indexbuffer;
	this->vertexbuffer = millingMaterial.vertexbuffer;

	this->gridX = millingMaterial.gridX;
	this->gridY = millingMaterial.gridY;
	this->vertices = millingMaterial.vertices;//?
}

void MillingMaterial::GetIndicesOfArea(XMFLOAT3 position, float range, int & left, int & right, int & top, int & down)
{
	//left = 0;
	//right = gridX - 1;
	//down = 0;
	//top = gridZ - 1;
	//return;

	float leftF = floorf((position.x - range + size.x / 2) / size.x * (gridX - 1));
	left = max(min((int)leftF, (gridX - 1)), 0);

	float rightF = ceilf((position.x + range + size.x / 2) / size.x * (gridX - 1));
	right = max(min((int)rightF, (gridX - 1)), 0);

	float downF = floorf((position.y - range + size.y / 2) / size.y * (gridY - 1));
	down = max(min((int)downF, (gridY - 1)), 0);

	float topF = ceilf((position.y + range + size.y / 2) / size.y * (gridY - 1));
	top = max(min((int)topF, (gridY - 1)), 0);
}

void MillingMaterial::Draw()
{
	UINT offset = 0;
	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer.GetAddressOf(), this->vertexbuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(this->indexbuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexbuffer.IndexCount(), 0, 0);
}

Vertex3D & MillingMaterial::GetVert(int x, int y)
{
	return vertices[x * gridY + y];
}

void MillingMaterial::Initialize(XMFLOAT3 size, int _gridX, int _gridY)
{
	this->gridX = _gridX;
	this->gridY = _gridY;
	this->size = size;

	Reset();
}

void MillingMaterial::Reset()
{
	this->vertices.clear();
	this->vertices.reserve(gridX * gridY);
	for (int i = 0; i < gridX; i++)
		for (int j = 0; j < gridY; j++)
		{
			float x = size.x * (i - (gridX - 1) / 2.0) / (gridX - 1);
			float y = size.y * (j - (gridY - 1) / 2.0) / (gridY - 1);
			float z = size.z;
			this->vertices.push_back(Vertex3D(x, y, z, 0, 0, 1));
		}

	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size(), true);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	std::vector<DWORD> indices;
	indices.reserve(6 * (gridX - 1) * (gridY - 1));
	for (int i = 0; i < gridX - 1; i++)
		for (int j = 0; j < gridY - 1; j++) {
			indices.push_back(gridY*(i + 1) + j + 1);
			indices.push_back(gridY*i + j + 1);
			indices.push_back(gridY*i + j);

			indices.push_back(gridY*(i + 1) + j);
			indices.push_back(gridY*(i + 1) + j + 1);
			indices.push_back(gridY*i + j);
		}

	hr = this->indexbuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

void MillingMaterial::UpdateVertexBuffer()
{
	D3D11_MAPPED_SUBRESOURCE resource;
	auto hr = deviceContext->Map(vertexbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	COM_ERROR_IF_FAILED(hr, "Failed to map vertex buffer.");

	memcpy(resource.pData, vertices.data(), vertices.size() * sizeof(Vertex3D));
	deviceContext->Unmap(vertexbuffer.Get(), 0);
}
