#include "PathGenerator.h"

PathGenerator::PathGenerator(MillingMaterial* _material)
{
	material = _material;
	minZ = 15;
	safeZ = material->size.z + 20;

	GenerateThirdPath();
}

void PathGenerator::SavePath(vector<Vector3> moves, string filePath)
{
	stringstream ss;

	for (int i = 0; i < moves.size(); i++)
	{
		Vector3& point = moves[i];
		bool x = i == 0 || point.x != moves[i - 1].x;
		bool y = i == 0 || point.y != moves[i - 1].y;
		bool z = i == 0 || point.z != moves[i - 1].z;

		if (!x && !y && !z)
			continue;

		ss << "N9G01";
		if (x) ss << "X" << fixed << std::setprecision(3) << point.x;
		if (y) ss << "Y" << fixed << std::setprecision(3) << point.y;
		if (z) ss << "Z" << fixed << std::setprecision(3) << point.z;
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

void PathGenerator::EnsureInit() {
	if (elephant.model0.surfaces.empty())
		elephant.LoadElephant(minZ);

	GenerateHeightMap();
}
void PathGenerator::GenerateHeightMap()
{
	heightMap.resize(material->gridX);
	for (int i = 0; i < material->gridX; i++)
		heightMap[i] = vector<float>(material->gridY, minZ);

	Matrix highMapTransform = XMMatrixTranslation(material->size.x / 2, material->size.y / 2, 0) * XMMatrixScaling(material->gridX / material->size.x, material->gridY / material->size.y, 1);

	//vector<BezierSurfaceC0*> surfaces = elephant.model0.GetSurfaces();
	vector<BezierSurfaceC0*> surfaces = elephant.model8.GetSurfaces();
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

				int x = static_cast<int>(roundf(point.x));
				int y = static_cast<int>(roundf(point.y));

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

void PathGenerator::GenerateFirstPath()
{
	EnsureInit();
	vector<Vector3> moves = GenerateFirstPathLayer((material->size.z + minZ) / 2);
	vector<Vector3> moves2 = GenerateFirstPathLayer(minZ + 0.1);
	moves.insert(moves.end(), moves2.begin(), moves2.end());
	SavePath(moves, "Paths\\elephant\\1.k16");
}
float PathGenerator::GetZ(float cpx, float cpy, bool flat, float toolRadius)
{
	float rangeSq = toolRadius * toolRadius;
	Vector3 currentPosition{ cpx, cpy, 0 };

	int left, right, top, down;
	material->GetIndicesOfArea(currentPosition, toolRadius, left, right, top, down);

	float result = minZ;

	for (int i = left; i < right + 1; i++)
	{
		for (int j = down; j < top + 1; j++)
		{
			float z = heightMap[i][j];

			if (!flat) {
				Vector3 pos = material->GetVert(i, j).pos;
				float x = currentPosition.x - pos.x;
				float y = currentPosition.y - pos.y;

				float distSq = x * x + y * y;
				float zoff = toolRadius - sqrt(rangeSq - y * y);
				z -= zoff;
			}

			result = max(result, z);
		}
	}

	return result;
}
vector<Vector3> PathGenerator::GenerateFirstPathLayer(float layerZ)
{
	vector<Vector3> path;

	//k16
	float xoff = 4;
	float yoff = 8;

	Vector2 bound = { material->size.x / 2, material->size.y / 2 };

	path.push_back({ -bound.x, -bound.y - yoff, safeZ });

	float x, y;
	bool reversed = false;
	for (x = -bound.x; x < bound.x + xoff; x += xoff)
	{
		vector<Vector3> subPath;

		float prevZ = layerZ;
		subPath.push_back({ x, -bound.y - yoff, layerZ });

		for (y = -bound.y; y < bound.y + yoff; y += yoff)
		{
			float z = max(layerZ, GetZ(x, y, false, 8));

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

		subPath.push_back({ x, y, layerZ });

		if (reversed)
			path.insert(path.end(), subPath.rbegin(), subPath.rend());
		else
			path.insert(path.end(), subPath.begin(), subPath.end());

		reversed = !reversed;
	}

	path.push_back({ x, y, safeZ });


	return path;
}

void PathGenerator::GenerateSecondPath()
{
	EnsureInit();
	vector<Vector3> moves = GenerateFlatLayer();
	vector<Vector3> moves2 = GenerateFlatEnvelope();
	moves.insert(moves.end(), moves2.begin(), moves2.end());
	SavePath(moves, "Paths\\elephant\\2.f10");
}
vector<Vector3> PathGenerator::GenerateFlatLayer()
{
	vector<Vector3> path;

	//f10
	float xoff = 3;
	float yoff = 2;
	float safeY = 8;

	Vector2 bound = { material->size.x / 2, material->size.y / 2 };

	path.push_back(Vector3(-bound.x, -bound.y - safeY, safeZ));
	path.push_back(Vector3(-bound.x, -bound.y - safeY, minZ));

	bool reversed = false;
	float x, y;
	for (x = -bound.x; x < bound.x + xoff; x += xoff)
	{
		for (y = -bound.y; y < bound.y + yoff; y += yoff)
		{
			float z = GetZ(x, y, true, 5);
			if (z != minZ)
				break;
		}
		y -= xoff;

		if (reversed)
		{
			Vector3 prev = path[path.size() - 1];
			Vector3 vert(x, y, minZ);

			if (prev.y > vert.y)
				path.push_back(Vector3(prev.x, y, minZ));
			else
				path.push_back(Vector3(x, prev.y, minZ));

			path.push_back(Vector3(x, y, minZ));
			path.push_back(Vector3(x, -bound.y - safeY, minZ));
		}
		else
		{
			path.push_back(Vector3(x, -bound.y - safeY, minZ));
			path.push_back(Vector3(x, y, minZ));
		}

		reversed = !reversed;

	}

	path.push_back({ x, y, safeZ });
	path.push_back(Vector3(-bound.x, bound.y + safeY, safeZ));
	path.push_back(Vector3(-bound.x, bound.y + safeY, minZ));

	reversed = false;
	for (x = -bound.x; x < bound.x + xoff; x += xoff)
	{
		for (y = bound.y; y > -bound.y - yoff; y -= yoff)
		{
			float z = GetZ(x, y, true, 5);
			if (z != minZ)
				break;
		}
		y += xoff;

		if (reversed)
		{
			Vector3 prev = path[path.size() - 1];
			Vector3 vert(x, y, minZ);

			if (prev.y < vert.y)
				path.push_back(Vector3(prev.x, y, minZ));
			else
				path.push_back(Vector3(x, prev.y, minZ));

			path.push_back(Vector3(x, y, minZ));
			path.push_back(Vector3(x, bound.y + safeY, minZ));
		}
		else
		{
			path.push_back(Vector3(x, bound.y + safeY, minZ));
			path.push_back(Vector3(x, y, minZ));
		}

		reversed = !reversed;
	}

	path.push_back({ x, y, safeZ });

	return path;
}
vector<Vector3> PathGenerator::GenerateFlatEnvelope()
{
	vector<Vector3> result;
	ModelVersion& model = this->elephant.model0;

	vector<Vector3> legBack[3];
	legBack[0] = GenerateUnrestrictedPath(model.GetLegBack(), { -75,0,minZ });
	legBack[1] = GenerateUnrestrictedCylinderPath(model.GetLegBack(), false);
	legBack[2] = GenerateUnrestrictedPath(model.GetLegBack(), { -20,-60,minZ });

	vector<Vector3> legFront[3];
	legFront[0] = GenerateUnrestrictedPath(model.GetLegFront(), { -20,-61,minZ });
	legFront[1] = GenerateUnrestrictedCylinderPath(model.GetLegFront(), false);
	legFront[2] = GenerateUnrestrictedPath(model.GetLegFront(), { 30,-60,minZ });

	vector<Vector3> torso[2];
	torso[0] = GenerateUnrestrictedPath(model.GetTorso(), { 50,-25,minZ });
	torso[1] = GenerateUnrestrictedPath(model.GetTorso(), { 50,25,minZ });

	vector<Vector3> head[4];
	head[0] = GenerateUnrestrictedPath(model.GetHead(), { 0,0,minZ });
	head[1] = GenerateUnrestrictedCylinderPath(model.GetHead(), false);
	head[2] = GenerateUnrestrictedPath(model.GetHead(), { 75,0,minZ });
	head[3] = GenerateUnrestrictedCylinderPath(model.GetHead(), true);

	vector<Vector3> box[3];
	box[0] = GenerateUnrestrictedCylinderPath(model.GetBox(), true);
	box[1] = GenerateUnrestrictedPath(model.GetBox(), { 0,50,minZ });
	box[2] = GenerateUnrestrictedCylinderPath(model.GetBox(), false);

	vector<Vector3> tail[3];
	tail[0] = GenerateUnrestrictedPath(model.GetTail(), { -50,-25,minZ });
	tail[1] = GenerateUnrestrictedCylinderPath(model.GetTail(), false);
	tail[2] = GenerateUnrestrictedPath(model.GetTail(), { -50,25,minZ });


	{
		auto tail_tmp = tail[2];
		TrimStart(torso[1], tail_tmp);
		result.insert(result.end(), tail_tmp.begin(), tail_tmp.end());

		result.insert(result.end(), tail[1].begin(), tail[1].end());

		tail_tmp = tail[0];
		TrimEnd(tail_tmp, torso[1]);
		result.insert(result.end(), tail_tmp.begin(), tail_tmp.end());
	}

	{
		auto torso_tmp = torso[0];
		TrimStart(tail[2], torso_tmp);
		TrimEnd(torso_tmp, legBack[0]);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

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
		TrimEnd(torso_tmp, head[0]);
		TrimStart(legFront[2], torso_tmp);
		result.insert(result.end(), torso_tmp.begin(), torso_tmp.end());
	}

	{
		auto head_tmp = head[0];
		TrimStart(torso[0], head_tmp);
		result.insert(result.end(), head_tmp.begin(), head_tmp.end());

		result.insert(result.end(), head[1].begin(), head[1].end());
		result.insert(result.end(), head[2].begin(), head[2].end());

		head_tmp = head[3];
		TrimStart(torso[1], head_tmp);
		result.insert(result.end(), head_tmp.rbegin(), head_tmp.rend());
	}

	//---

	{
		auto box_tmp = box[0];
		TrimStart(torso[1], box_tmp);
		result.insert(result.end(), box_tmp.begin(), box_tmp.end());

		result.insert(result.end(), box[1].begin(), box[1].end());

		box_tmp = box[2];
		TrimStart(torso[1], box_tmp);
		result.insert(result.end(), box_tmp.rbegin(), box_tmp.rend());
	}

	Vector3 start1 = result[0];
	start1.y = material->size.y / 2 + 20;

	Vector3 start2 = start1;
	start2.z = safeZ;

	result.insert(result.begin(), start1);
	result.insert(result.begin(), start2);
	result.push_back(start1);
	result.push_back(start2);

	return result;
}
BezierSurfaceC0 PathGenerator::GetPlane()
{
	Vector3 s = material->size;
	BezierSurfaceC0 plane(1, 1);
	for (int w = 0; w < 4; w++)
		for (int h = 0; h < 4; h++)
			plane.GetVert(w, h) = Vector3(s.x * w / 3 - s.x / 2, s.y * h / 3 - s.y / 2, minZ);

	return plane;
}
vector<Vector3> PathGenerator::GenerateUnrestrictedPath(BezierSurfaceC0* surface, Vector3 startingPoint)
{
	float toolRadius = 5.0f;
	float filterDist = 1.0f;
	vector<Vector3> result;
	BezierSurfaceC0 plane = GetPlane();
	IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ &plane, surface }, startingPoint, 0.0001);

	if (curve != nullptr) {
		for (int i = 0; i < curve->_uv1.size(); i++)
		{
			Vector2 uv = curve->_uv1[i];
			Vector3 position = surface->Evaluate(uv);
			Vector3 normal = surface->EvaluateNormal(uv);
			position.z = minZ;
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
vector<Vector3> PathGenerator::GenerateUnrestrictedCylinderPath(BezierSurfaceC0* surface, bool top)
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
			vert1.z = minZ;
			vert2.z = minZ;

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
	vert1.z = minZ;
	vert2.z = minZ;
	vert1Deep.z = minZ;
	vert2Deep.z = minZ;

	Vector3 normal;
	if (Vector3::Distance(vert2, vert1) > 0.0001f)
		normal = (vert2 - vert1).Cross(Vector3(0, 0, 1));
	else
		normal = vert1Deep + vert2Deep;

	normal.z = 0;
	normal.Normalize();

	if (normal.Dot(vert1 - vert1Deep) < 0)
		normal = -normal;

	Vector3 normal1 = surface->EvaluateNormal(uv1);
	Vector3 normal2 = surface->EvaluateNormal(uv2);
	normal1.z = 0;
	normal2.z = 0;
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

void PathGenerator::GenerateThirdPath()
{
	EnsureInit();
	vector<Vector3> moves = GenerateSurfacePaths();
	vector<Vector3> moves2 = GenerateSurfaceIntersectionPaths();
	moves.insert(moves.end(), moves2.begin(), moves2.end());
	SavePath(moves, "Paths\\elephant\\3.k08");
}
vector<Vector3> PathGenerator::GenerateSurfaceIntersectionPaths()
{
	vector<Vector3> result, tmp, tmp2;
	ModelVersion& model = elephant.model8;

	/*tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-5, 20, minZ + 10));
	AddSafe(tmp);
	result.insert(result.end(), tmp.begin(), tmp.end());

	tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-10, 20, minZ + 10));
	AddSafe(tmp);
	result.insert(result.end(), tmp.begin(), tmp.end());

	tmp = GenerateUnrestrictedPath(model.GetTail(), model.GetTorso(), Vector3(-50, 10, minZ));
	tmp.erase(tmp.end() - 2, tmp.end());
	AddSafe(tmp);
	result.insert(result.end(), tmp.rbegin(), tmp.rend());

	tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, 20, minZ + 10));
	AddSafe(tmp);
	result.insert(result.end(), tmp.rbegin(), tmp.rend());

	tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	tmp.erase(tmp.end() - 127, tmp.end());
	AddSafe(tmp);
	result.insert(result.end(), tmp.rbegin(), tmp.rend());

	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetTorso(), Vector3(70, 0, minZ+10));
	AddSafe(tmp);
	result.insert(result.end(), tmp.begin(), tmp.end());

	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(20, 20, minZ + 10));

	tmp2 = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(50, 20, minZ + 10));
	tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());

	tmp2 = GenerateUnrestrictedPath(model.GetTorso(), model.GetRightEar(), Vector3(20, 20, minZ + 10));
	tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());

	AddSafe(tmp);
	result.insert(result.end(), tmp.begin(), tmp.end());*/

	return result;
}
vector<Vector3> PathGenerator::GenerateUnrestrictedPath(BezierSurfaceC0* surface1, BezierSurfaceC0* surface2, Vector3 startingPoint)
{
	float toolRadius = 4.0f;
	float filterDist = 1.0f;
	vector<Vector3> result;
	IntersectionCurve* curve = IntersectionCurve::FindIntersectionCurve({ surface1, surface2 }, startingPoint, 0.0001);

	if (curve != nullptr) {
		for (int i = 0; i < curve->Verts.size(); i++)
		{
			Vector3 position = curve->Verts[i];
			position.z -= toolRadius;

			bool add = false;
			if (position.z > minZ)
			{
				add = true;
			}
			else if (i > 0 && curve->Verts[i - 1].z > minZ || i<curve->Verts.size() - 1 && curve->Verts[i + 1].z > minZ)
			{
				position.z = minZ;
				add = true;
			}

			if (add)
				if (result.size() == 0 || Vector3::Distance(result[result.size() - 1], position) > filterDist)
					result.push_back(position);
		}
		delete curve;
	}

	return result;
}
void PathGenerator::AddSafe(vector<Vector3>& path)
{
	AddSafe(path, safeZ);
}
void PathGenerator::AddSafe(vector<Vector3>& path, float safeH)
{
	AddSafeStart(path, safeH);
	AddSafeEnd(path, safeH);
}
void PathGenerator::AddSafeStart(vector<Vector3>& path)
{
	AddSafeStart(path, safeZ);
}
void PathGenerator::AddSafeStart(vector<Vector3>& path, float safeH)
{
	path.insert(path.begin(), path[0]);
	path[0].z = safeH;

}
void PathGenerator::AddSafeEnd(vector<Vector3>& path)
{
	AddSafeEnd(path, safeZ);
}
void PathGenerator::AddSafeEnd(vector<Vector3>& path, float safeH)
{
	path.push_back(*path.rbegin());
	path.rbegin()->z = safeH;

}
vector<Vector3> PathGenerator::GenerateSurfacePaths()
{
	vector<Vector3> result, tmp, tmp2;
	vector<vector<Vector3>> tmp3;
	ModelVersion& model = elephant.model8;

	/*tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	tmp.erase(tmp.end() - 127, tmp.end());
	tmp3 = AddParametrizationLine(model.GetLegFront(), true);
	for (int i = 0; i < tmp3.size(); i++)
	{
		auto& p = tmp3[i];
		TrimEnd2(p, tmp);
		AddSafe(p, 30);
		if (i == 0) AddSafeStart(p);
		if (i == tmp3.size() - 1) AddSafeEnd(p);
		result.insert(result.begin(), p.begin(), p.end());
	}

	tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, 20, minZ + 10));
	tmp3 = AddParametrizationLine(model.GetLegBack(), true);
	for (int i = 0; i < tmp3.size(); i++)
	{
		auto& p = tmp3[i];
		TrimEnd2(p, tmp);
		AddSafe(p, 30);
		if (i == 0) AddSafeStart(p);
		if (i == tmp3.size() - 1) AddSafeEnd(p);
		result.insert(result.begin(), p.begin(), p.end());
	}*/


	//tmp = GenerateUnrestrictedPath(model.GetTail(), model.GetTorso(), Vector3(-50, 10, minZ));
	//tmp.erase(tmp.end() - 2, tmp.end());
	//tmp[0].y -= 10;
	//tmp[tmp.size() - 1].y += 10;
	//tmp3 = AddParametrizationLine(model.GetTail(), true);
	//for (int i = 0; i < tmp3.size(); i++)
	//{
	//	auto& p = tmp3[i];
	//	TrimEnd2(p, tmp);
	//	AddSafe(p, 30);
	//	if (i == 0) AddSafeStart(p);
	//	if (i == tmp3.size() - 1) AddSafeEnd(p);
	//	result.insert(result.begin(), p.begin(), p.end());
	//}

	//tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-5, 20, minZ + 10));
	//tmp2 = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-10, 20, minZ + 10));
	//tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());
	//tmp3 = AddParametrizationLine(model.GetBox(), false,true);
	//for (int i = 0; i < tmp3.size(); i++)
	//{
	//	auto& p = tmp3[i];
	//	TrimStart2(tmp, p);
	//	AddSafe(p, 40);
	//	if (i == 0) AddSafeStart(p);
	//	if (i == tmp3.size() - 1) AddSafeEnd(p);
	//	result.insert(result.begin(), p.begin(), p.end());
	//}


	/*tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, 20, minZ + 10));
	tmp3 = AddParametrizationLine(model.GetLegBack(), true);
	for (int i = 0; i < tmp3.size(); i++)
	{
		auto& p = tmp3[i];
		TrimEnd2(p, tmp);
		AddSafe(p, 30);
		if (i == 0) AddSafeStart(p);
		if (i == tmp3.size() - 1) AddSafeEnd(p);
		result.insert(result.begin(), p.begin(), p.end());
	}*/

	return result;
}
vector<vector<Vector3>> PathGenerator::AddParametrizationLine(BezierSurfaceC0* surface, bool wdir, bool notZero)
{
	vector<vector<Vector3>> result;
	float toolRadius = 4.0f;
	float probesCount = 50;

	for (int i = 0; i < probesCount; i++)
	{
		float ti = 1.0f * i / probesCount;
		vector<Vector3 > path;

		bool anyZ = false;
		for (int j = 0; j < probesCount; j++)
		{
			float tj = 1.0f * j / probesCount;

			Vector2 hw = wdir ? Vector2(tj, ti) : Vector2(ti, tj);
			Vector3 pos = surface->Evaluate(hw);
			pos.z -= toolRadius;

			if (pos.z >= minZ)
			{
				anyZ = true;
			}
			else
			{
				pos.z = minZ;
			}

			if (!notZero || pos.z > minZ)
				path.push_back(pos);
		}

		if (anyZ)
			result.push_back(path);
	}

	return result;
}

void PathGenerator::TrimStart2(vector<Vector3>& trimmer, vector<Vector3>& path)
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
				break;
			}
		}
	}
}
void PathGenerator::TrimEnd2(vector<Vector3>& path, vector<Vector3>& trimmer)
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
				break;
			}
		}
	}
}
void PathGenerator::AddOuterSafe(vector<vector<Vector3>>& paths)
{
	if (paths.empty()) return;

	AddSafeStart(paths[0]);
	AddSafeEnd(paths[paths.size()-1]);
}
void PathGenerator::AddInnerSafe(vector<vector<Vector3>>& paths, float height)
{
	for (int i = 0; i < paths.size(); i++)
	{
		auto& p = paths[i];
		if (p.empty()) continue;

		if (i > 0)
			AddSafeStart(p, height);

		if (i < paths.size() - 1)
			AddSafeEnd(p, height);
	}

}
void PathGenerator::TrimEnd3(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer)
{
	for (auto& p : paths)
		TrimEnd3(p, trimmer);
}
void PathGenerator::TrimStart3(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer)
{
	for (auto& p : paths)
		TrimStart3(p, trimmer);
}
void PathGenerator::TrimCenter(vector<vector<Vector3>>& paths, vector<Vector3>& trimmerFrom, vector<Vector3>& trimmerTo)
{
	for (auto& p : paths)
		TrimCenter(p, trimmerFrom, trimmerTo);
}
void PathGenerator::TrimEnd3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int from = FindIntersectionLast(path, trimmer) + 1;
	path.erase(path.begin() + from, path.end());
}
void PathGenerator::TrimStart3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int to = FindIntersection(path, trimmer) + 1;
	path.erase(path.begin(), path.end() + to);
}
void PathGenerator::TrimCenter(vector<Vector3>& path, vector<Vector3>& trimmerFrom, vector<Vector3>& trimmerTo)
{
	int from = FindIntersection(path, trimmerFrom) + 1;
	int to = FindIntersection(path, trimmerTo) + 1;
	path.erase(path.begin() + from, path.begin() + to);
}
int PathGenerator::FindIntersection(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int idx1, int idx2;

	if (FindIntersection(path, trimmer, idx1, idx2))
		return idx1;
	else
		return -1;
}
int PathGenerator::FindIntersectionLast(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int idx1, int idx2;

	if (FindIntersectionLast(path, trimmer, idx1, idx2))
		return idx1;
	else
		return -1;
}
bool PathGenerator::FindIntersection(vector<Vector3>& path, vector<Vector3>& trimmer, int& idx1, int& idx2)
{
	for (int i = 0; i < path.size() - 1; i++)
	{
		for (int j = 0; j < trimmer.size() - 1; j++)
		{
			Vector2 out;
			Vector2 seg1_a = Vector2(path[i].x, path[i].y);
			Vector2 seg1_b = Vector2(path[i + 1].x, path[i + 1].y);
			Vector2 seg2_a = Vector2(trimmer[j].x, trimmer[j].y);
			Vector2 seg2_b = Vector2(trimmer[j + 1].x, trimmer[j + 1].y);

			if (SegmentsIntersect(seg1_a, seg1_b, seg2_a, seg2_b, out)) {
				idx1 = i;
				idx2 = j;
				return true;
			}
		}
	}
	return false;
}
bool PathGenerator::FindIntersectionLast(vector<Vector3>& path, vector<Vector3>& trimmer, int& idx1, int& idx2)
{
	for (int i = path.size() - 1; i > 0; i--)
	{
		for (int j = 0; j < trimmer.size() - 1; j++)
		{
			Vector2 out;
			Vector2 seg1_a = Vector2(path[i].x, path[i].y);
			Vector2 seg1_b = Vector2(path[i - 1].x, path[i - 1].y);
			Vector2 seg2_a = Vector2(trimmer[j].x, trimmer[j].y);
			Vector2 seg2_b = Vector2(trimmer[j + 1].x, trimmer[j + 1].y);

			if (SegmentsIntersect(seg1_a, seg1_b, seg2_a, seg2_b, out)) {
				idx1 = i - 1;
				idx2 = j;
				return true;
			}
		}
	}
	return false;
}