#include "PathGenerator.h"

PathGenerator::PathGenerator(MillingMaterial* _material)
{
	material = _material;
	GeneratePaths();//qwe
}


void PathGenerator::SavePath(vector<Vector3> moves, string filePath)
{
	stringstream ss;

	for (int i = 0; i < moves.size(); i++)
	{
		ss << "N9G01";

		Vector3& point = moves[i];

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
		heightMap[i] = vector<float>(material->gridY, 0);

	Matrix highMapTransform = XMMatrixTranslation(material->size.x / 2, material->size.y / 2, 0) * XMMatrixScaling(material->gridX / material->size.x, material->gridY / material->size.y, 1);


	vector<BezierSurfaceC0*> surfaces = model.GetSurfaces();
	for (int k = 0; k < surfaces.size(); k++)
	{
		BezierSurfaceC0* surf = surfaces[k];
		for (int i = 0; i < material->gridX; i++)
		{
			for (int j = 0; j < material->gridY; j++)
			{
				float u = 1.0f * i / material->gridX;
				float v = 1.0f * j / material->gridY;
				Vector3 point = surf->Evaluate(Vector2(u, v));

				point = Vector3::Transform(point, highMapTransform);

				int x = static_cast<int>(point.x);
				int y = static_cast<int>(point.y);

				if (x < material->gridX && y < material->gridY && x >= 0 && y >= 0)
					heightMap[x][y] = (max(heightMap[x][y], point.z));
			}
		}
	}

	for (int i = 0; i < material->gridX; i++)
		for (int j = 0; j < material->gridY; j++)
			material->GetVert(i, j).pos.z = heightMap[i][j];

	material->UpdateVertexBuffer();
}

float PathGenerator::GetZ(float cpx, float cpy, bool flat)
{
	float cutRadius = 8;
	float rangeSq = cutRadius * cutRadius;
	Vector3 currentPosition{ cpx, cpy, 0 };

	int left, right, top, down;
	material->GetIndicesOfArea(currentPosition, cutRadius, left, right, top, down);

	float result = 0;

	for (int i = left; i < right + 1; i++)
	{
		for (int j = down; j < top + 1; j++)
		{
			Vector3 pos = material->GetVert(i, j).pos;
			float x = currentPosition.x - pos.x;
			float y = currentPosition.y - pos.y;

			float z = heightMap[i][j];

			if (!flat) {
				float distSq = x * x + y * y;
				float zoff = cutRadius - sqrt(rangeSq - y * y);
				z -= zoff;
			}

			result = max(result, z);
		}
	}

	return result;
}

vector<Vector3> PathGenerator::GenerateFirstPathLayer(float minZ)
{
	vector<Vector3> path;

	//k16
	float xoff = 4;
	float yoff = 8;
	float safeZ = material->size.z + 20;

	Vector2 bound = { material->size.x / 2, material->size.y / 2 };

	path.push_back({ -bound.x, -bound.y - yoff, safeZ });

	float x, y;
	bool reversed = false;
	for (x = -bound.x; x < bound.x + xoff; x += xoff)
	{
		vector<Vector3> subPath;

		float prevZ = 0;
		subPath.push_back({ x, -bound.y - yoff, minZ });

		for (y = -bound.y; y < bound.y + yoff; y += yoff)
		{
			float z = max(minZ, GetZ(x, y, false));

			if (prevZ == z)
				continue;

			if (z < prevZ) {
				subPath.push_back({ x, y , prevZ });
				subPath.push_back({ x, y , z });
			}
			if (z > prevZ) {
				subPath.push_back({ x, y - yoff , prevZ });
				subPath.push_back({ x, y - yoff , z });
			}

			prevZ = z;
		}

		subPath.push_back({ x, y, minZ });

		if (reversed)
			path.insert(path.end(), subPath.rbegin(), subPath.rend());
		else
			path.insert(path.end(), subPath.begin(), subPath.end());

		reversed = !reversed;
	}

	path.push_back({ x, y, safeZ });


	return path;
}

void PathGenerator::GeneratePaths()
{
	model.LoadElephant();

	GenerateHeightMap();
	//vector<Vector3> moves = GenerateFirstPathLayer(5);
	//vector<Vector3> moves2 = GenerateFirstPathLayer(0);
	//moves.insert(moves.end(), moves2.begin(), moves2.end());
	//SavePath(moves, "Paths\\elephant\\1.k16");

	vector<Vector3> moves = GenerateFlatLayer(0);
	vector<Vector3> moves2 = GenerateFlatEnvelope(0);
	moves.insert(moves.end(), moves2.begin(), moves2.end());
	SavePath(moves, "Paths\\elephant\\2.f10");
}



vector<Vector3> PathGenerator::GenerateFlatLayer(float minZ)
{
	return vector<Vector3>();
}

vector<Vector3> PathGenerator::GenerateFlatEnvelope(float minZ)
{
	Vector3 s = material->size;
	BezierSurfaceC0 plane(1, 1);
	for (int w = 0; w < 4; w++)
		for (int h = 0; h < 4; h++)
			plane.GetVert(w, h) = Vector3(s.x * w / 3 - s.x / 2, s.y * h / 3 - s.y / 2, 0);

	/*BezierSurfaceC0 plane2(2, 2, true);
	for (int h = 0; h < 7; h++)
	{
		plane2.GetVert(0, h) = Vector3(0, 25, (h - 1) * 10);
		plane2.GetVert(1, h) = Vector3(25, 25, (h - 1) * 10);
		plane2.GetVert(2, h) = Vector3(25, -25, (h - 1) * 10);
		plane2.GetVert(3, h) = Vector3(0, -25, (h - 1) * 10);
		plane2.GetVert(4, h) = Vector3(-25, -25, (h - 1) * 10);
		plane2.GetVert(5, h) = Vector3(-25, 25, (h - 1) * 10);
		plane2.GetVert(6, h) = Vector3(0, 25, (h - 1) * 10);
	}*/

	vector<Vector3> result;
	IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, model.GetTorso() }, { -25,-25,0 }, 0.03);
	//IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ model[0].get(), model[1].get() }, { -2.916667f, - 5.333333f,0 }, 0.02);
	if (curve != nullptr) {
		result.insert(result.end(), curve->Verts.begin(), curve->Verts.end());
		delete curve;
	}

	//vector<Vector3> result;
	//for (int i = 0; i < model.size(); i++) {
	//	BezierSurfaceC0* surf = model[i].get();
	//	//IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, surf }, { 20,20,20 }, 0.01);
	//	//IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, surf }, { -100,-50,0 }, 0.01);
	//	IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, surf }, { 75,-30,0 }, 0.01);
	//	if (curve != nullptr) {
	//		result.insert(result.end(), curve->Verts.begin(), curve->Verts.end());
	//		delete curve;
	//	}
	//}


	return result;
}