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
	vector<Vector3> result;

	vector<Vector3> legBack[3];
	legBack[0] = GenerateUnrestrictedPath(model.GetLegBack(), { -75,0,0 });
	legBack[1] = GenerateUnrestrictedCylinderPath(model.GetLegBack(), false, 0);
	legBack[2] = GenerateUnrestrictedPath(model.GetLegBack(), { -20,-60,0 });

	vector<Vector3> legFront[3];
	legFront[0] = GenerateUnrestrictedPath(model.GetLegFront(), { -20,-60,0 });
	legFront[1] = GenerateUnrestrictedCylinderPath(model.GetLegFront(), false, 0);
	legFront[2] = GenerateUnrestrictedPath(model.GetLegFront(), { 30,-60,0 });

	vector<Vector3> torso[2];
	torso[0] = GenerateUnrestrictedPath(model.GetTorso(), { 0,-25,0 });
	torso[1] = GenerateUnrestrictedPath(model.GetTorso(), { 0,25,0 });

	vector<Vector3> head[4];
	head[0] = GenerateUnrestrictedPath(model.GetHead(), { 0,0,0 });
	head[1] = GenerateUnrestrictedCylinderPath(model.GetHead(), false, 0);
	head[2] = GenerateUnrestrictedPath(model.GetHead(), { 75,0,0 });
	head[3] = GenerateUnrestrictedCylinderPath(model.GetHead(), true, 0);

	vector<Vector3> box[3];
	box[0] = GenerateUnrestrictedCylinderPath(model.GetBox(), true, 0);
	box[1] = GenerateUnrestrictedPath(model.GetBox(), { 0,50,0 });
	box[2] = GenerateUnrestrictedCylinderPath(model.GetBox(), false, 0);

	vector<Vector3> tail[3];
	tail[0] = GenerateUnrestrictedPath(model.GetTail(), { 0,-25,0 });
	tail[1] = GenerateUnrestrictedCylinderPath(model.GetTail(), false, 0);
	tail[2] = GenerateUnrestrictedPath(model.GetTail(), { 0,25,0 });


	{
		auto legBack_tmp = legBack[0];
		TrimStart(torso[0], legBack_tmp);
		result.insert(result.end(), legBack_tmp.begin(), legBack_tmp.end());

		result.insert(result.end(), legBack[1].begin(), legBack[1].end());

		legBack_tmp = legBack[2];
		TrimEnd(legBack_tmp, torso[0]);
		result.insert(result.end(), legBack_tmp.begin(), legBack_tmp.end());
	}

	{
		auto torso_tmp = torso[0];
		TrimStart(legBack[2], torso_tmp);
		TrimEnd(torso_tmp, legFront[0]);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

	{
		auto legFront_tmp = legFront[0];
		TrimStart(torso[0], legFront_tmp);
		result.insert(result.end(), legFront_tmp.begin(), legFront_tmp.end());

		result.insert(result.end(), legFront[1].begin(), legFront[1].end());

		legFront_tmp = legFront[2];
		TrimEnd(legFront_tmp, torso[0]);
		result.insert(result.end(), legFront_tmp.begin(), legFront_tmp.end());
	}

	{
		auto torso_tmp = torso[0];
		TrimStart(legFront[2], torso_tmp);
		TrimEnd(torso_tmp, head[0]);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

	{
		auto head_tmp = head[0];
		TrimStart(torso[0], head_tmp);
		result.insert(result.end(), head_tmp.begin(), head_tmp.end());

		result.insert(result.end(), head[1].begin(), head[1].end());
		result.insert(result.end(), head[2].begin(), head[2].end());

		head_tmp = head[3];
		TrimStart(torso[1],head_tmp);
		result.insert(result.end(), head_tmp.rbegin(), head_tmp.rend());
	}

	////---

	{
		auto box_tmp = box[0];
		TrimStart(torso[1], box_tmp);
		result.insert(result.end(), box_tmp.begin(), box_tmp.end());

		result.insert(result.end(), box[1].begin(), box[1].end());

		box_tmp = box[2];
		TrimStart(torso[1], box_tmp);
		result.insert(result.end(), box_tmp.rbegin(), box_tmp.rend());
	}

	{
		auto torso_tmp = torso[1];
		TrimStart(box[2], torso_tmp);
		TrimEnd(torso_tmp, tail[0]);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

	{
		auto tail_tmp = tail[0];
		TrimStart(torso[1], tail_tmp);
		result.insert(result.end(), tail_tmp.begin(), tail_tmp.end());

		result.insert(result.end(), tail[1].begin(), tail[1].end());

		tail_tmp = tail[2];
		TrimEnd(tail_tmp, torso[0]);
		result.insert(result.end(), tail_tmp.begin(), tail_tmp.end());
	}

	{
		auto torso_tmp = torso[0];
		TrimStart(tail[2], torso_tmp);
		TrimEnd(torso_tmp, legBack[0]);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

	return result;
}

BezierSurfaceC0 PathGenerator::GetPlane(float z)
{
	Vector3 s = material->size;
	BezierSurfaceC0 plane(1, 1);
	for (int w = 0; w < 4; w++)
		for (int h = 0; h < 4; h++)
			plane.GetVert(w, h) = Vector3(s.x * w / 3 - s.x / 2, s.y * h / 3 - s.y / 2, z);

	return plane;
}

vector<Vector3> PathGenerator::GenerateUnrestrictedPath(BezierSurfaceC0* surface, Vector3 startingPoint)
{
	float toolRadius = 5.0f;
	float filterDist = 1.0f;
	vector<Vector3> result;
	BezierSurfaceC0 plane = GetPlane(0);
	IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, surface }, startingPoint, 0.03);

	if (curve != nullptr) {
		for (int i = 0; i < curve->_uv1.size(); i++)
		{
			Vector2 uv = curve->_uv1[i];
			Vector3 position = surface->Evaluate(uv);
			Vector3 normal = surface->EvaluateNormal(uv);
			normal.z = 0;
			normal.Normalize();
			position += toolRadius * normal;

			if (i == 0 || i == (int)(curve->_uv1.size()) - 1
				|| Vector3::Distance(result[result.size() - 1], position) > filterDist)
				result.push_back(position);
		}

		delete curve;
	}

	RemoveSelfIntersections(result);

	return result;
}
vector<Vector3> PathGenerator::GenerateUnrestrictedCylinderPath(BezierSurfaceC0* surface, bool top, float z)
{
	vector<Vector3> result;
	if (!surface->isCylinder)
		return result;

	float u = top ? 1.0f : 0.0f;
	float uBack = top ? 0.9f : 0.1f;
	int probes_count = 100;

	Vector2 uv1;
	Vector2 uv2;
	float dist = 0;
	float toolRange = 5.0f;

	for (int i = 0; i < probes_count; i++)
	{
		for (int j = i + 1; j < probes_count; j++) {
			float v1 = i / 100.0f;
			float v2 = j / 100.0f;

			Vector3 vert1 = surface->Evaluate(Vector2(u, v1));
			Vector3 vert2 = surface->Evaluate(Vector2(u, v2));
			vert1.z = z;
			vert2.z = z;

			float new_dist = Vector3::Distance(vert1, vert2);
			if (new_dist > dist) {
				dist = new_dist;
				uv1 = Vector2(u, v1);
				uv2 = Vector2(u, v2);
			}
		}
	}

	Vector3 vert1 = surface->Evaluate(uv1);
	Vector3 vert2 = surface->Evaluate(uv2);
	Vector3 vert1Deep = surface->Evaluate(Vector2(uBack, uv1.y));
	Vector3 vert2Deep = surface->Evaluate(Vector2(uBack, uv2.y));
	vert1.z = 0;
	vert2.z = 0;
	vert1Deep.z = 0;
	vert2Deep.z = 0;

	Vector3 normal;
	if (Vector3::Distance(vert2, vert1) > 0.0001f)
		normal = (vert2 - vert1).Cross(Vector3(0, 0, 1));
	else
		normal = vert1Deep + vert2Deep;

	normal.Normalize();

	if (normal.Dot(vert1 - vert1Deep) < 0)
		normal = -normal;

	Vector3 normal1 = surface->EvaluateNormal(uv1);
	Vector3 normal2 = surface->EvaluateNormal(uv2);
	normal1.z = z;
	normal2.z = z;
	normal1.Normalize();
	normal2.Normalize();

	result.push_back(vert1 + toolRange * (normal1 + normal));
	result.push_back(vert2 + toolRange * (normal2 + normal));

	return result;
}
bool PathGenerator::SegmentsIntersect(const Vector2& A, const Vector2& B, const Vector2& C, const Vector2& D, Vector2& out)
{
	Vector2 CmP = C - A;
	Vector2 r = B - A;
	Vector2 s = D - C;

	float CmPxr = CmP.x * r.y - CmP.y * r.x;
	float CmPxs = CmP.x * s.y - CmP.y * s.x;
	float rxs = r.x * s.y - r.y * s.x;

	if (CmPxr == 0.0f)
	{
		return ((C.x - A.x < 0.0f) != (C.x - B.x < 0.0f)) || ((C.y - A.y < 0.0f) != (C.y - B.y < 0.0f));
	}

	if (rxs == 0.0f)
		return false; // Lines are parallel.

	float rxsr = 1.0f / rxs;
	float t = CmPxs * rxsr;
	float u = CmPxr * rxsr;

	out = A + t * r;

	return (t >= 0.0f) && (t <= 1.0f) && (u >= 0.0f) && (u <= 1.0f);
}

void PathGenerator::RemoveSelfIntersections(vector<Vector3>& path)
{
	while (true) {
		bool changed = false;

		for (int offset = 2; offset < path.size() - 2; offset++)
		{
			for (int i = 0; i < path.size() - 2 - offset; i++)
			{
				Vector2 out;
				Vector2 seg1_a = Vector2(path[i].x, path[i].y);
				Vector2 seg1_b = Vector2(path[i + 1].x, path[i + 1].y);
				Vector2 seg2_a = Vector2(path[i + offset].x, path[i + offset].y);
				Vector2 seg2_b = Vector2(path[i + offset + 1].x, path[i + offset + 1].y);

				if (SegmentsIntersect(seg1_a, seg1_b, seg2_a, seg2_b, out)) {
					path.erase(path.begin() + i + 1, path.begin() + i + offset + 1);
					path.insert(path.begin() + i + 1, Vector3(out.x, out.y, minZ));
					changed = true;
					break;
				}
			}

			if (changed)
				break;
		}

		if (!changed)
			break;
	}
}

void PathGenerator::TrimStart(vector<Vector3>& trimmer, vector<Vector3>& path)
{
	for (int i = 0; i < trimmer.size() - 1; i++)
	{
		for (int j = 0; j < path.size() - 1; j++)
		{
			Vector2 out;
			Vector2 seg1_a = Vector2(trimmer[i].x, trimmer[i].y);
			Vector2 seg1_b = Vector2(trimmer[i + 1].x, trimmer[i + 1].y);
			Vector2 seg2_a = Vector2(path[j].x, path[j].y);
			Vector2 seg2_b = Vector2(path[j + 1].x, path[j + 1].y);

			if (SegmentsIntersect(seg1_a, seg1_b, seg2_a, seg2_b, out)) {
				path.erase(path.begin(), path.begin() + j);
				path[0] = Vector3(out.x, out.y, minZ);
				break;
			}
		}
	}
}

void PathGenerator::TrimEnd(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	for (int i = trimmer.size() - 1; i > 0; i--)
	{
		for (int j = 0; j < path.size() - 1; j++)
		{
			Vector2 out;
			Vector2 seg1_a = Vector2(trimmer[i].x, trimmer[i].y);
			Vector2 seg1_b = Vector2(trimmer[i - 1].x, trimmer[i - 1].y);
			Vector2 seg2_a = Vector2(path[j].x, path[j].y);
			Vector2 seg2_b = Vector2(path[j + 1].x, path[j + 1].y);

			if (SegmentsIntersect(seg1_a, seg1_b, seg2_a, seg2_b, out)) {
				path.erase(path.begin() + j + 1, path.end());
				path[path.size() - 1] = Vector3(out.x, out.y, minZ);
				break;
			}
		}
	}
}
