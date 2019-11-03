#include "MillingMachine.h"

MillingMachine::MillingMachine(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	device = _device;
	deviceContext = _deviceContext;

	safePosition = XMFLOAT3(0, 0, 120);
	stepSize = 1;
	speed = 1;
	materialDepth = 50;
	toolDepth = 50;
	materialDepthViolated = false;
	toolDepthViolated = false;
}

void MillingMachine::LoadDataFromFile(string filePath)
{
	string extension = filePath.substr(filePath.rfind("."));
	bool flatCut = extension[1] == 'f';
	float cutRadius = stoi(extension.substr(2)) / 2.0;

	SetMillingCutterMesh(cutRadius, flatCut);

	moves.clear();
	currentMove = 0;
	currentPosition = safePosition;
	finished = false;

	ifstream file(filePath);
	string str;
	while (getline(file, str)) {
		if (str[0] != 'N')
			continue;

		int pos = str.find("G");
		if (pos == -1)
			continue;

		if (str.substr(pos + 1, 2) != "01")
			continue;

		XMFLOAT3 position;
		str = str.substr(pos + 3);

		if (str[0] == 'X') {
			pos = str.find(".");
			position.x = stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.x = moves.back().x;
		}

		if (str[0] == 'Y') {
			pos = str.find(".");
			position.y = stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.y = moves.back().y;
		}

		if (str[0] == 'Z') {
			pos = str.find(".");
			position.z = stof(str.substr(1, pos + 4));
			str = str.substr(pos + 4);
		}
		else {
			position.z = moves.back().z;
		}


		moves.push_back(position);
	}
	moves.insert(moves.begin(), safePosition);
	moves.push_back(safePosition);

	SetPathMesh();
}

void MillingMachine::SetMillingCutterMesh(float radius, bool flat)
{
	flatCut = flat;
	cutRadius = radius;
	float r = radius;
	int horizontalLvls = 10;
	int roundLvls = 10;
	float height = 300;
	float startHeight = flat ? 0 : r;

	vector<Vertex3D> vertices;
	vector<DWORD> indices;

	for (int i = 0; i < horizontalLvls; i++)
	{
		float angle = XM_2PI * i / horizontalLvls;
		float angle2 = XM_2PI * (i + 1) / horizontalLvls;
		XMFLOAT3 a = Normalize(XMFLOAT3(cos(angle), sin(angle), 0));
		XMFLOAT3 b = Normalize(XMFLOAT3(cos(angle2), sin(angle2), 0));

		int count = vertices.size();
		vertices.push_back(Vertex3D(r * b.x, r * b.y, startHeight, b.x, b.y, 0));
		vertices.push_back(Vertex3D(r * b.x, r * b.y, height, b.x, b.y, 0));
		vertices.push_back(Vertex3D(r * a.x, r * a.y, height, a.x, a.y, 0));
		vertices.push_back(Vertex3D(r * a.x, r * a.y, startHeight, a.x, a.y, 0));

		indices.push_back(count); indices.push_back(count + 1); indices.push_back(count + 2);
		indices.push_back(count); indices.push_back(count + 2); indices.push_back(count + 3);
	}

	if (!flat)
	{
		for (int i = 0; i < horizontalLvls; i++)
		{
			float angle = XM_2PI * i / horizontalLvls;
			float angle2 = XM_2PI * (i + 1) / horizontalLvls;

			for (int j = 0; j < roundLvls; j++)
			{
				float roundAngle = XM_PIDIV2 * j / roundLvls;
				float roundAngle2 = XM_PIDIV2 * (j + 1) / roundLvls;

				XMFLOAT3 a = Normalize(XMFLOAT3(cos(angle) * sin(roundAngle), sin(angle) * sin(roundAngle), cos(roundAngle)));
				XMFLOAT3 a2 = Normalize(XMFLOAT3(cos(angle) * sin(roundAngle2), sin(angle) * sin(roundAngle2), cos(roundAngle2)));
				XMFLOAT3 b = Normalize(XMFLOAT3(cos(angle2) * sin(roundAngle), sin(angle2) * sin(roundAngle), cos(roundAngle)));
				XMFLOAT3 b2 = Normalize(XMFLOAT3(cos(angle2) * sin(roundAngle2), sin(angle2) * sin(roundAngle2), cos(roundAngle2)));

				int count = vertices.size();
				vertices.push_back(Vertex3D(r * b.x, r * b.y, r * (1 - b.z), b2.x, b.y, b.z));
				vertices.push_back(Vertex3D(r * b2.x, r * b2.y, r * (1 - b2.z), b2.x, b2.y, b2.z));
				vertices.push_back(Vertex3D(r * a2.x, r * a2.y, r * (1 - a2.z), a2.x, a2.y, a2.z));
				vertices.push_back(Vertex3D(r * a.x, r * a.y, r * (1 - a.z), a.x, a.y, a.z));

				indices.push_back(count); indices.push_back(count + 1); indices.push_back(count + 2);
				indices.push_back(count); indices.push_back(count + 2); indices.push_back(count + 3);
			}
		}
	}

	vector<Texture> textures;
	XMMATRIX mtx = XMMatrixTranslation(safePosition.x, safePosition.y, safePosition.z);

	millingCutterMesh = shared_ptr<Mesh>(new Mesh(device, deviceContext, vertices, indices, textures, mtx));
}

void MillingMachine::SetPathMesh()
{
	vector<Vertex3D> vertices;
	vector<DWORD> indices;

	float width = 0.4;

	for (int i = 0; i < (int)moves.size() - 1; i++)
	{
		int count = vertices.size();
		XMFLOAT3 right = { width / 2,0,0 };
		if (moves[i].x != moves[i + 1].x || moves[i].z != moves[i + 1].z) {
			XMVECTOR dir = (XMLoadFloat3(&moves[i + 1]) - XMLoadFloat3(&moves[i]));
			XMStoreFloat3(&right, (width / 2) * XMVector3Normalize(XMVector3Cross({ 0,1,0 }, dir)));
		}

		vertices.push_back(Vertex3D(moves[i].x - right.x, moves[i].y, moves[i].z - right.z, 0, 1, 0));
		vertices.push_back(Vertex3D(moves[i + 1].x - right.x, moves[i + 1].y, moves[i + 1].z - right.z, 0, 1, 0));
		vertices.push_back(Vertex3D(moves[i + 1].x + right.x, moves[i + 1].y, moves[i + 1].z + right.z, 0, 1, 0));
		vertices.push_back(Vertex3D(moves[i].x + right.x, moves[i].y, moves[i].z + right.z, 0, 1, 0));

		indices.push_back(count); indices.push_back(count + 1); indices.push_back(count + 2);
		indices.push_back(count); indices.push_back(count + 2); indices.push_back(count + 3);
		indices.push_back(count + 3); indices.push_back(count + 2); indices.push_back(count + 1);
		indices.push_back(count + 3); indices.push_back(count + 1); indices.push_back(count + 0);
	}

	vector<Texture> textures;
	XMMATRIX mtx = XMMatrixIdentity();

	pathMesh = shared_ptr<Mesh>(new Mesh(device, deviceContext, vertices, indices, textures, mtx));
}

void MillingMachine::Reset()
{
	materialDepthViolated = false;
	toolDepthViolated = false;
	millingViolated = false;
	restTime = 0;
	currentPosition = moves[0];
}

void MillingMachine::Update(float dt, MillingMaterial* material)
{
	restTime += dt;
	float timePerStep = stepSize / speed;
	while (!finished && restTime > timePerStep) {
		XMFLOAT3 dir = Move();
		Cut(dir, material);
		restTime -= timePerStep;
	}

	millingCutterMesh->transformMatrix = XMMatrixTranslation(currentPosition.x, currentPosition.y, currentPosition.z);
}

XMFLOAT3 MillingMachine::Move()
{
	XMVECTOR a = XMLoadFloat3(&currentPosition);
	XMVECTOR b = XMLoadFloat3(&moves[currentMove]);
	XMVECTOR toEndMove = b - a;

	XMVECTOR dir = XMVector3Normalize(toEndMove);
	XMVECTOR movement = dir * stepSize;

	XMFLOAT3 moveLen, toEndLen;
	XMStoreFloat3(&moveLen, XMVector3Length(movement));
	XMStoreFloat3(&toEndLen, XMVector3Length(toEndMove));

	if (moveLen.x < toEndLen.x) {
		XMStoreFloat3(&currentPosition, XMLoadFloat3(&currentPosition) + movement);
	}
	else {
		currentPosition = moves[currentMove];

		if (moves.size() > currentMove + 1)
			currentMove++;
		else
			finished = true;
	}

	XMFLOAT3 dirFloat;
	XMStoreFloat3(&dirFloat, dir);

	return dirFloat;
}

void MillingMachine::Cut(XMFLOAT3 dir, MillingMaterial* material)
{
	float eps = 0.0000000;
	bool millingDanger = flatCut && dir.y < -eps;

	float rangeSq = cutRadius * cutRadius;

	int left, right, top, down;
	material->GetIndicesOfArea(currentPosition, cutRadius, left, right, top, down);

	//update heights
	for (int i = left; i < right + 1; i++)
	{
		for (int j = down; j < top + 1; j++)
		{
			XMFLOAT3 pos = material->GetVert(i, j).pos;
			float x = currentPosition.x - pos.x;
			float z = currentPosition.z - pos.z;

			float distSq = x * x + z * z;
			if (distSq < rangeSq)
			{
				float newHeight = flatCut ? currentPosition.y : currentPosition.y + cutRadius - sqrt(rangeSq - distSq);
				if (pos.y > newHeight)
				{
					if (newHeight < material->size.y - materialDepth)
						materialDepthViolated = true;

					if (pos.y - newHeight > toolDepth)
						toolDepthViolated = true;

					if (millingDanger)
						millingViolated = true;

					pos.y = newHeight;
				}

				material->GetVert(i, j).pos = pos;
			}
		}
	}


	left = max(0, left - 1);
	right = min(material->gridX - 1, right + 1);
	down = max(0, down - 1);
	top = min(material->gridZ - 1, top + 1);

	//update normals
	for (int i = left; i < right + 1; i++)
	{
		for (int j = down; j < top + 1; j++)
		{
			XMFLOAT3 curr = material->GetVert(i, j).pos;

			XMFLOAT3 left = i == 0 ? curr : material->GetVert(i - 1, j).pos;
			XMFLOAT3 right = i == material->gridX - 1 ? curr : material->GetVert(i + 1, j).pos;
			XMFLOAT3 top = j == 0 ? curr : material->GetVert(i, j - 1).pos;
			XMFLOAT3 down = j == material->gridZ - 1 ? curr : material->GetVert(i, j + 1).pos;

			XMFLOAT3 normal = CalculateNormal(left, right, top, down);
			material->GetVert(i, j).normal = normal;
		}
	}

	material->UpdateVertexBuffer();
}

XMFLOAT3 MillingMachine::CalculateNormal(const XMFLOAT3& left, const XMFLOAT3& right, const XMFLOAT3& top, const XMFLOAT3& down)
{
	XMVECTOR vecX = XMLoadFloat3(&XMFLOAT3(right.x - left.x, right.y - left.y, right.z - left.z));
	XMVECTOR vecZ = XMLoadFloat3(&XMFLOAT3(top.x - down.x, top.y - down.y, top.z - down.z));
	XMVECTOR vecNormal = XMVector3Normalize(XMVector3Cross(vecX, vecZ));
	XMFLOAT3 normal;
	XMStoreFloat3(&normal, vecNormal);

	return normal;
}

XMFLOAT3 MillingMachine::Normalize(XMFLOAT3 v)
{
	XMVECTOR vec = XMLoadFloat3(&v);
	XMFLOAT3 normalized;
	XMStoreFloat3(&normalized, XMVector3Normalize(vec));
	return normalized;
}
