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
	this->gridZ = millingMaterial.gridZ;
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

	float downF = floorf((position.z - range + size.z / 2) / size.z * (gridZ - 1));
	down = max(min((int)downF, (gridZ - 1)), 0);

	float topF = ceilf((position.z + range + size.z / 2) / size.z * (gridZ - 1));
	top = max(min((int)topF, (gridZ - 1)), 0);
}

void MillingMaterial::Draw()
{
	UINT offset = 0;
	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer.GetAddressOf(), this->vertexbuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(this->indexbuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexbuffer.IndexCount(), 0, 0);
}

Vertex3D & MillingMaterial::GetVert(int x, int z)
{
	return vertices[x * gridZ + z];
}

void MillingMaterial::Initialize(XMFLOAT3 size, int _gridX, int _gridZ)
{
	this->gridX = _gridX;
	this->gridZ = _gridZ;
	this->size = size;

	Reset();
}

void MillingMaterial::Reset()
{
	this->vertices.clear();
	this->vertices.reserve(gridX * gridZ);
	for (int i = 0; i < gridX; i++)
		for (int j = 0; j < gridZ; j++)
		{
			float x = size.x * (i - (gridX - 1) / 2.0) / (gridX - 1);
			float y = size.y;
			float z = size.z * (j - (gridZ - 1) / 2.0) / (gridZ - 1);
			this->vertices.push_back(Vertex3D(x, y, z, 0, 1, 0));
		}

	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size(), true);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	std::vector<DWORD> indices;
	indices.reserve(6 * (gridX - 1) * (gridZ - 1));
	for (int i = 0; i < gridX - 1; i++)
		for (int j = 0; j < gridZ - 1; j++) {
			indices.push_back(gridZ*i + j);
			indices.push_back(gridZ*i + j + 1);
			indices.push_back(gridZ*(i + 1) + j + 1);

			indices.push_back(gridZ*i + j);
			indices.push_back(gridZ*(i + 1) + j + 1);
			indices.push_back(gridZ*(i + 1) + j);
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

void MillingMaterial::Randomize()
{
	static int frame = 0;
	frame++;

	for (int i = 0; i < gridX; i++)
		for (int j = 0; j < gridZ; j++) {
			float angle = XM_2PI * (i * 10 + frame) / 100;
			GetVert(i, j).pos.y = std::sin(angle);
		}
}
