#include "PathGenerator.h"

PathGenerator::PathGenerator(MillingMaterial* _material)
{
	material = _material;
}

vector<BezierSurfaceC0*> PathGenerator::GetModel()
{
	vector<BezierSurfaceC0*> result;

	for (int i = 0; i < model.size(); i++)
		result.push_back(model[i].get());

	return result;
}

void PathGenerator::LoadElephant()
{
	model.clear();

	char cCurrentPath[FILENAME_MAX];
	_getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = std::string(cCurrentPath) + "\\Models\\";
	string filePath = path + "wt_elephant.mg1";

	ifstream file(filePath);
	string line;
	while (getline(file, line)) {

		vector<string> header;
		StringHelper::Split(line, header);

		if (header.empty())
			continue;

		string elementName = header[0];
		int n = stoi(header[1]);

		for (int j = 0; j < n; j++)
		{
			getline(file, line);

			if (elementName == "surfaceC0")
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line)));
			else if (elementName == "tubeC0")
				model.push_back(make_shared<BezierSurfaceC0>(BezierSurfaceC0(line, true)));
		}
	}

	XMMATRIX modelTransform = XMMatrixScaling(material->gridX / 35.0f, material->gridY / 35.0f, 3) * XMMatrixTranslation(material->gridX / 2.25f, material->gridY / 2.25f, -3);

	for (int k = 0; k < model.size(); k++)
	{
		BezierSurfaceC0* surf = model[k].get();
		for (int i = 0; i < surf->GetWidthVertexCount(); i++)
		{
			for (int j = 0; j < surf->GetHeightVertexCount(); j++)
			{
				XMStoreFloat3(&surf->GetVert(i, j), XMVector3TransformCoord(XMLoadFloat3(&surf->GetVert(i, j)), modelTransform));
			}
		}
	}

}
void PathGenerator::SavePath(vector<XMFLOAT3> moves, string filePath)
{
	stringstream ss;

	for (int i = 0; i < moves.size(); i++)
	{
		ss << "N9G01";

		XMFLOAT3& point = moves[i];

		if (i == 0 || point.x != moves[i - 1].x)
			ss << "X" << fixed << std::setprecision(3) << point.x;

		if (i == 0 || point.y != moves[i - 1].y)
			ss << "Y" << fixed << std::setprecision(3) << point.y;

		if (i == 0 || point.z != moves[i - 1].z)
			ss << "Z" << fixed << std::setprecision(3) << point.z;

		ss << endl;
	}

	ofstream file(filePath);
	if (file.is_open())
	{
		file.clear();
		file << ss.str();

		file.close();
	}
}

void PathGenerator::GenerateHeightMap()
{
	heightMap.resize(material->gridX);
	for (int i = 0; i < material->gridX; i++)
		heightMap[i] = vector<float>(material->gridY, 0.0f);

	//for (int i = 0; i < material->gridX; i++)
	//	for (int j = 0; j < material->gridY; j++)
	//		material->GetVert(i, j).pos.z = 0;

	for (int k = 0; k < model.size(); k++)
	{
		BezierSurfaceC0* surf = model[k].get();
		for (int i = 0; i < material->gridX; i++)
		{
			for (int j = 0; j < material->gridY; j++)
			{
				float u = 1.0f * i / material->gridX;
				float v = 1.0f * j / material->gridY;
				XMFLOAT3 point = surf->Evaluate(XMFLOAT2(u, v));


				int x = static_cast<int>(point.x);
				int y = static_cast<int>(point.y);

				if (x < material->gridX && y < material->gridY && x >= 0 && y >= 0)
					heightMap[x][y] = max(heightMap[x][y], -point.z);
			}

		}

	}

	//for (int i = 0; i < material->gridX; i++)
	//	for (int j = 0; j < material->gridY; j++)
	//		material->GetVert(i, j).pos.z = heightMap[i][j];

	//material->UpdateVertexBuffer();
}

float PathGenerator::GetHighestZ(float x, float y)
{
	float cutRadius = 8;
	float rangeSq = cutRadius * cutRadius;
	XMFLOAT3 currentPosition{ x, y, 0 };

	int left, right, top, down;
	material->GetIndicesOfArea(currentPosition, cutRadius, left, right, top, down);

	float result = 0;
	//update heights
	for (int i = left; i < right + 1; i++)
	{
		for (int j = down; j < top + 1; j++)
		{
			XMFLOAT3 pos = material->GetVert(i, j).pos;
			float x = currentPosition.x - pos.x;
			float y = currentPosition.y - pos.y;

			float distSq = x * x + y * y;

			float z = heightMap[i][j];
			float zoff = cutRadius - sqrt(rangeSq - y * y);

			result = max(result, z - zoff);
		}
	}

	return result;
}

vector<XMFLOAT3> PathGenerator::GenerateFirstPath()
{
	vector<XMFLOAT3> path;

	//k16
	float xoff = 6;
	float yoff = 8;
	float safeZ = material->size.z + 20;

	XMFLOAT2 bound = { material->size.x / 2, material->size.y / 2 };

	XMFLOAT3 start = { -bound.x, -bound.y - yoff, safeZ };
	path.push_back(start);

	float x, y;
	bool reversed = false;
	for (x = -bound.x; x < bound.x + xoff; x += xoff)
	{
		vector<XMFLOAT3> subPath;

		float z = 0;
		XMFLOAT3 ystart = { x, -bound.y - yoff, z };
		subPath.push_back(ystart);

		for (y = -bound.y; y < bound.y + yoff; y += yoff)
		{
			z = GetHighestZ(x, y);
			XMFLOAT3 point = { x, y, z };
			subPath.push_back(point);
		}

		XMFLOAT3 end = { x, y, z };
		subPath.push_back(end);

		if (reversed)
			path.insert(path.end(), subPath.rbegin(), subPath.rend());
		else
			path.insert(path.end(), subPath.begin(), subPath.end());

		reversed = !reversed;
	}

	XMFLOAT3 end = { x, y, safeZ };
	path.push_back(end);


	return path;
}

void PathGenerator::GeneratePaths()
{
	LoadElephant();

	GenerateHeightMap();
	vector<XMFLOAT3> moves = GenerateFirstPath();
	SavePath(moves, "Paths\\elephant\\test1.k16");
}