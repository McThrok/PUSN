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

	vector<BezierSurfaceC0*> surfaces = elephant.model0.GetSurfaces();
	//vector<BezierSurfaceC0*> surfaces = elephant.model8.GetSurfaces();
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

	GenerateSecondPath();
	GenerateThirdPath();
}
float PathGenerator::GetZ(float cpx, float cpy, float toolRadius)
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
			float z = max(layerZ, GetZ(x, y, 8) + 4);

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
			float z = GetZ(x, y, 5);
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
			float z = GetZ(x, y, 5);
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
	vector<Vector3> result, tmp;
	ModelVersion& model = this->elephant.model0;

	vector<Vector3> legBack[3];
	legBack[0] = GenerateUnrestrictedPath(model.GetLegBack(), { -75,0,minZ });
	legBack[1] = GenerateUnrestrictedCylinderPath(model.GetLegBack(), false);
	legBack[2] = GenerateUnrestrictedPath(model.GetLegBack(), { -20,-60,minZ });

	vector<Vector3> legFront[3];
	legFront[0] = GenerateUnrestrictedPath(model.GetLegFront(), { -20,-61,minZ });
	legFront[1] = GenerateUnrestrictedCylinderPath(model.GetLegFront(), false);
	legFront[2] = GenerateUnrestrictedPath(model.GetLegFront(), { 30,-60,minZ });

	vector<Vector3> torso[4];
	torso[0] = GenerateUnrestrictedPath(model.GetTorso(), { 50,-25,minZ });
	torso[1] = GenerateUnrestrictedCylinderPath(model.GetTorso(), false);
	torso[2] = GenerateUnrestrictedPath(model.GetTorso(), { 50,25,minZ });
	torso[3] = GenerateUnrestrictedCylinderPath(model.GetTorso(), true);
	reverse(torso[1].begin(), torso[1].end());

	vector<Vector3> head[4];
	head[0] = GenerateUnrestrictedPath(model.GetHead(), { 0,0,minZ });
	head[1] = GenerateUnrestrictedCylinderPath(model.GetHead(), false);
	head[2] = GenerateUnrestrictedPath(model.GetHead(), { 75,0,minZ });
	head[3] = GenerateUnrestrictedCylinderPath(model.GetHead(), true);
	reverse(head[3].begin(), head[3].end());

	vector<Vector3> box[3];
	box[0] = GenerateUnrestrictedCylinderPath(model.GetBox(), true);
	box[1] = GenerateUnrestrictedPath(model.GetBox(), { 0,50,minZ });
	box[2] = GenerateUnrestrictedCylinderPath(model.GetBox(), false);
	reverse(box[2].begin(), box[2].end());

	vector<Vector3> tail[3];
	tail[0] = GenerateUnrestrictedPath(model.GetTail(), { -50,25,minZ });
	tail[1] = GenerateUnrestrictedCylinderPath(model.GetTail(), false);
	tail[2] = GenerateUnrestrictedPath(model.GetTail(), { -50,-25,minZ });


	{
		tmp = torso[3];
		TrimEnd3(tmp, tail[0]);
		Append(result, tmp);
	}

	{
		tmp = tail[0];
		TrimStart3(tmp, torso[3]);
		Append(result, tmp);

		Append(result, tail[1]);

		tmp = tail[2];
		TrimEnd3(tmp, torso[0]);
		Append(result, tmp);
	}

	{
		tmp = torso[0];
		TrimStart3(tmp, tail[2]);
		TrimEnd3(tmp, legBack[0]);
		Append(result, tmp);
	}

	{
		tmp = legBack[0];
		TrimStart3(tmp, torso[0]);
		Append(result, tmp);

		Append(result, legBack[1]);

		tmp = legBack[2];
		TrimEnd3(tmp, torso[0]);
		Append(result, tmp);
	}

	{
		tmp = torso[0];
		TrimStart3(tmp, legBack[2]);
		TrimEnd3(tmp, legFront[0]);
		Append(result, tmp);
	}

	{
		tmp = legFront[0];
		TrimStart3(tmp, torso[0]);
		Append(result, tmp);

		Append(result, legFront[1]);

		tmp = legFront[2];
		TrimEnd3(tmp, torso[0]);
		Append(result, tmp);
	}

	{
		tmp = torso[0];
		TrimStart3(tmp, legFront[2]);
		Append(result, tmp);

		tmp = torso[1];
		TrimEnd3(tmp, head[0]);
		Append(result, tmp);
	}

	{
		tmp = head[0];
		TrimStart3(tmp, torso[1]);
		Append(result, tmp);

		Append(result, head[1]);
		Append(result, head[2]);

		tmp = head[3];
		TrimEnd3(tmp, torso[2]);
		Append(result, tmp);
	}

	{
		tmp = box[0];
		TrimStart3(tmp, torso[2]);
		Append(result, tmp);

		Append(result, box[1]);

		tmp = box[2];
		TrimEnd3(tmp, torso[2]);
		Append(result, tmp);
	}

	{
		tmp = torso[2];
		TrimStart3(tmp, box[2]);
		Append(result, tmp);
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
			position.z = minZ;
			if (toolRadius != 0)
			{
				Vector3 normal = surface->EvaluateNormal(uv);
				normal.z = 0;
				normal.Normalize();

				position += toolRadius * normal;
			}

			if (i == 0 || i == (int)(curve->_uv1.size()) - 1
				|| Vector3::Distance(result[result.size() - 1], position) > filterDist)
				result.push_back(position);
		}

		delete curve;
	}

	RemoveSelfIntersections(result);

	return result;
}
vector<Vector3> PathGenerator::GenerateUnrestrictedCylinderPath(BezierSurfaceC0* surface, bool top, float toolRange)
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
	normal1 += normal - normal * normal1.Dot(normal);
	normal2 += normal - normal * normal2.Dot(normal);

	int probesCount = 100;
	Vector3 v1 = vert1 + toolRange * normal1;
	Vector3 v2 = vert2 + toolRange * normal2;
	for (int i = 0; i < probesCount; i++)
		result.push_back(v1 + (Vector3)((v2 - v1) * i * 1.0f / (probesCount - 1)));

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

void PathGenerator::GenerateThirdPath()
{
	EnsureInit();
	vector<Vector3> moves = GenerateSurfacePaths();
	//vector<Vector3> moves2 = GenerateSurfaceIntersectionPaths();
	//Append(moves, moves2);
	SavePath(moves, "Paths\\elephant\\3.k08");
}
vector<Vector3> PathGenerator::Test()
{
	vector<Vector3> result, tmp, tmp2;
	vector<vector<Vector3>> tmp3;
	ModelVersion& model = elephant.model8;
	tmp3 = AddParametrizationLine(model.GetLegFront(), false, false);
	Append(result, tmp3);

	return result;
}
vector<Vector3> PathGenerator::GenerateSurfaceIntersectionPaths()
{
	vector<Vector3> result, tmp, tmp2;
	vector<vector<Vector3>> tmp3;
	ModelVersion& model = elephant.model8;

	tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-5, 20, minZ + 10));
	AddSafe(tmp);
	Append(result, tmp);

	tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-15, 20, minZ + 10));
	AddSafe(tmp);
	Append(result, tmp);

	tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, -20, minZ + 10));
	tmp.erase(tmp.end() - 165, tmp.end());
	AddSafe(tmp);
	Append(result, tmp);

	tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	tmp.erase(tmp.begin(), tmp.begin() + 332);
	AddSafe(tmp);
	Append(result, tmp);

	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetTorso(), Vector3(70, 0, minZ + 10));
	AddSafe(tmp);
	Append(result, tmp);

	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(20, 20, minZ + 10));
	tmp2 = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(50, 20, minZ + 10));
	Append(tmp, tmp2);
	tmp.push_back(tmp[0]);
	AddSafe(tmp);
	Append(result, tmp);

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
vector<Vector3> PathGenerator::GenerateSurfacePaths()
{
	vector<Vector3> result, tmp, tmp2;
	vector<vector<Vector3>> tmp3;
	ModelVersion& model = elephant.model8;

	////legs
	//tmp3 = AddParametrizationLine(model.GetLegFront(), true, false);
	//tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	//tmp.erase(tmp.begin(), tmp.begin() + 332);
	//tmp[0].x -= 10;
	//tmp.rbegin()->x += 10;
	//TrimEnd3(tmp3, tmp);
	//Finalize(tmp3, 30);
	//Append(result, tmp3);

	//tmp3 = AddParametrizationLine(model.GetLegBack(), true, false);
	//tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, -20, minZ + 10));
	//tmp.erase(tmp.end() - 165, tmp.end());
	//tmp[0].x -= 10;
	//tmp.rbegin()->x += 10;
	//TrimEnd3(tmp3, tmp);
	//Finalize(tmp3, 30);
	//Append(result, tmp3);

	////tail
	//tmp3 = AddParametrizationLine(model.GetTail(), true, false);
	//tmp = GenerateUnrestrictedPath(model.GetTail(), model.GetTorso(), { 50,-25,minZ });
	//TrimEnd3(tmp3, tmp);
	//tmp = GenerateUnrestrictedCylinderPath(model.GetTorso(), true, 0);
	//TrimEnd3(tmp3, tmp);
	//Finalize(tmp3, 30);
	//Append(result, tmp3);

	////box
	//tmp3 = AddParametrizationLine(model.GetBox(), true, false);
	//tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-5, 20, minZ + 10));
	//tmp[0].x += 10;
	//tmp.rbegin()->x -= 10;
	//TrimEnd3(tmp3, tmp);
	//tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-15, 20, minZ + 10));
	//tmp[0].x += 10;
	//tmp.rbegin()->x -= 10;
	//TrimStart3(tmp3, tmp);
	//Finalize(tmp3, 40);
	//Append(result, tmp3);

	//head
	tmp3 = AddParametrizationLine(model.GetHead(), true, false);
	tmp2 = GenerateUnrestrictedCylinderPath(model.GetTorso(), false, 0);
	tmp2.erase(tmp2.begin(), tmp2.begin() + tmp2.size() / 2);
	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetTorso(), Vector3(70, 0, minZ + 10));
	Append(tmp, tmp2);
	TrimEnd3(tmp3, tmp);
	tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(50, 20, minZ + 10));
	tmp2 = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(20, 20, minZ + 10));
	Append(tmp, tmp2);
	tmp.push_back(tmp[0]);
	TrimCenter(tmp3, tmp, tmp);
	Finalize(tmp3, 52);
	Append(result, tmp3);

	////ear
	//tmp3 = AddParametrizationLine(model.GetRightEar(), false, false);
	//tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(50, 20, minZ + 10));
	//TrimStart3(tmp3, tmp);
	//tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetRightEar(), Vector3(20, 20, minZ + 10));
	//TrimEnd3(tmp3, tmp);
	//Finalize(tmp3, 60);
	//Append(result, tmp3);

	//tmp3 = AddParametrizationLine(model.GetHead(), true, false);
	//tmp2 = GenerateUnrestrictedCylinderPath(model.GetTorso(), false, 0);
	//tmp2.erase(tmp2.begin(), tmp2.begin() + tmp2.size() / 2);
	//tmp = GenerateUnrestrictedPath(model.GetHead(), model.GetTorso(), Vector3(70, 0, minZ + 10));

	////torso
	//tmp3 = AddParametrizationLine(model.GetTorso(), true, false);
	//tmp = GenerateUnrestrictedCylinderPath(model.GetHead(), true, 0);
	//tmp.erase(tmp.begin(), tmp.begin() + tmp.size() / 2);
	//tmp2 = GenerateUnrestrictedPath(model.GetHead(), model.GetTorso(), Vector3(70, 0, minZ + 10));
	//Append(tmp, tmp2);
	//Append(result, tmp);
	//TrimStart3(tmp3, tmp);


	//tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, -20, minZ + 10));
	//tmp.erase(tmp.end() - 165, tmp.end());
	//tmp[0].y -= 10;
	//tmp.rbegin()->y -= 10;
	//TrimCenter(tmp3, tmp, tmp);

	//tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	//tmp.erase(tmp.begin(), tmp.begin() + 332);
	//tmp[0].y -= 10;
	//tmp.rbegin()->y -= 10;
	//TrimCenter(tmp3, tmp, tmp);

	//tmp = GenerateUnrestrictedPath(model.GetLegBack(), model.GetTorso(), Vector3(-50, -20, minZ + 10));
	//tmp.erase(tmp.end() - 165, tmp.end());
	//Append(result, tmp);
	//TrimCenter(tmp3, tmp, tmp);
	//tmp = GenerateUnrestrictedPath(model.GetLegFront(), model.GetTorso(), Vector3(60, -20, minZ));
	//tmp.erase(tmp.begin(), tmp.begin() + 332);
	//TrimCenter(tmp3, tmp, tmp);

	//tmp = GenerateUnrestrictedPath(model.GetTail(), model.GetTorso(), Vector3(50, -25, minZ));
	//tmp[0].y -= 10;
	//tmp[0].x += 10;
	//TrimEnd3(tmp3, tmp);

	//tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-5, 20, minZ + 10));
	//tmp[0].x += 5;
	//DuplicateFirst(tmp);
	//tmp[0].y += 10;
	//tmp.rbegin()->y += 10;
	//TrimCenter(tmp3, tmp, tmp);

	//tmp = GenerateUnrestrictedPath(model.GetTorso(), model.GetBox(), Vector3(-15, 20, minZ + 10));
	//tmp[0].y += 10;
	//tmp.rbegin()->x -= 5;
	//DuplicateLast(tmp);
	//tmp.rbegin()->y += 10;
	//TrimCenter(tmp3, tmp, tmp);

	//Finalize(tmp3, 50);
	//Append(result, tmp3);


	return result;
}
vector<vector<Vector3>> PathGenerator::AddParametrizationLine(BezierSurfaceC0* surface, bool wdir, bool notZero)
{
	vector<vector<Vector3>> result;
	float toolRadius = 4.0f;
	float probesCount = 200;
	Vector3 pos = surface->EvaluateNormal(Vector2(0.5, 0.75));

	for (int i = 0; i < probesCount; i++)
	{
		float ti = 1.0f * i / (probesCount - 1);
		vector<Vector3 > path;

		bool anyZ = false;
		for (int j = 0; j < probesCount; j++)
		{
			float tj = 1.0f * j / (probesCount - 1);

			Vector2 hw = wdir ? Vector2(tj, ti) : Vector2(ti, tj);
			Vector3 pos = surface->Evaluate(hw);
			//pos += toolRadius * surface->EvaluateNormal(hw);
			//pos.z -= toolRadius;

			if (pos.z >= minZ)
				anyZ = true;
			else
				pos.z = minZ;

			if (!notZero || pos.z > minZ)
				path.push_back(pos);
		}

		if (anyZ)
			result.push_back(path);
	}

	return result;
}

void PathGenerator::Reverse(vector<Vector3>& path)
{
	reverse(path.begin(), path.end());
}
void PathGenerator::Finalize(vector<vector<Vector3>>& path, float interHeight)
{
	RemoveEmpty(path);
	AddInnerSafe(path, interHeight);
	AddOuterSafe(path);
}
bool  PathGenerator::DuplicateFirst(vector<Vector3>& path)
{
	if (path.empty())
		return false;

	path.insert(path.begin(), path[0]);
	return true;

}
bool  PathGenerator::DuplicateLast(vector<Vector3>& path)
{
	if (path.empty())
		return false;

	path.push_back(*path.rbegin());
	return true;
}

void PathGenerator::Append(vector<Vector3>& path, vector<vector<Vector3>>& toAppend)
{
	for (int i = 0; i < toAppend.size(); i++)
		Append(path, toAppend[i]);
}
void PathGenerator::Append(vector<Vector3>& path, vector<Vector3>& toAppend)
{
	path.insert(path.end(), toAppend.begin(), toAppend.end());
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
	if (DuplicateFirst(path))
		path[0].z = safeH;
}
void PathGenerator::AddSafeEnd(vector<Vector3>& path)
{
	AddSafeEnd(path, safeZ);
}
void PathGenerator::AddSafeEnd(vector<Vector3>& path, float safeH)
{
	if (DuplicateLast(path))
		path.rbegin()->z = safeH;

}
void PathGenerator::RemoveEmpty(vector<vector<Vector3>>& paths)
{
	auto new_end = std::remove_if(paths.begin(), paths.end(),
		[](const vector<Vector3>& path)
		{ return path.size() == 0; });

	paths.erase(new_end, paths.end());
}


void PathGenerator::TrimEndLast3(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer)
{
	for (auto& p : paths)
		TrimEndLast3(p, trimmer);

}
void PathGenerator::TrimStartLast3(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer)
{
	for (auto& p : paths)
		TrimStartLast3(p, trimmer);

}
void PathGenerator::TrimEndLast3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int from = FindIntersection(path, trimmer);
	if (from >= 0)
		path.erase(path.begin() + from + 1, path.end());
}
void PathGenerator::TrimStartLast3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int to = FindIntersectionLast(path, trimmer);
	if (to >= 0)
		path.erase(path.begin(), path.begin() + to + 1);
}

void PathGenerator::AddOuterSafe(vector<vector<Vector3>>& paths)
{
	if (paths.empty()) return;

	AddSafeStart(paths[0]);
	AddSafeEnd(paths[paths.size() - 1]);
}
void PathGenerator::AddInnerSafe(vector<vector<Vector3>>& paths, float height)
{
	for (int i = 0; i < paths.size(); i++)
	{
		auto& p = paths[i];
		if (p.empty()) continue;

		if (i > 0)
			AddSafeStart(p, 10 + height);

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
	vector<vector<Vector3>> result;

	for (int i = 0; i < paths.size(); i++)
	{
		vector<Vector3> out;
		TrimCenter(paths[i], out, trimmerFrom, trimmerTo);

		if (!paths[i].empty())
			result.push_back(paths[i]);

		if (!out.empty())
			result.push_back(out);

		/*if (!paths[i].empty() && !out.empty())
		{
			paths[i].insert(paths[i].begin(), out.begin(), out.end());
		}
		else if (paths[i].empty())
		{
			paths.erase(paths.begin() + i);
			i--;
		}
		else if (!out.empty())
		{
			paths.insert(paths.begin() + i + 1, out);
			i++;
		}*/
	}
	paths = result;
}
void PathGenerator::TrimEnd3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int from = FindIntersectionLast(path, trimmer);
	if (from >= 0)
		path.erase(path.begin() + from + 1, path.end());
}
void PathGenerator::TrimStart3(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int to = FindIntersection(path, trimmer);
	if (to >= 0)
		path.erase(path.begin(), path.begin() + to + 1);
}
void PathGenerator::TrimCenter(vector<Vector3>& path, vector<Vector3>& pathOut, vector<Vector3>& trimmerFrom, vector<Vector3>& trimmerTo)
{
	int from = FindIntersection(path, trimmerFrom);
	int to = FindIntersectionLast(path, trimmerTo);


	if (from >= 0 && to == -1)
	{
		path.erase(path.begin() + from + 1, path.end());
	}
	else if (from == -1 && to >= 0)
	{
		pathOut = vector<Vector3>(path.begin() + to + 1, path.end());
		path.clear();
	}
	else if (from >= 0 && to >= 0)
	{
		pathOut = vector<Vector3>(path.begin() + to + 1, path.end());
		path.erase(path.begin() + from + 1, path.end());
	}
}
int PathGenerator::FindIntersection(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int idx1, idx2;

	if (FindIntersection(path, trimmer, idx1, idx2))
		return idx1;
	else
		return -1;
}
int PathGenerator::FindIntersectionLast(vector<Vector3>& path, vector<Vector3>& trimmer)
{
	int idx1, idx2;

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

void PathGenerator::TrimEnd(vector<Vector3>& path, vector<Vector3>& trimmer, float innerPoint)
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
				path[path.size() - 1] = Vector3(out.x, out.y, innerPoint);
				break;
			}
		}
	}
}
void PathGenerator::TrimStart(vector<Vector3>& path, vector<Vector3>& trimmer, float innerPoint)
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
				path[0] = Vector3(out.x, out.y, innerPoint);
				break;
			}
		}
	}
}
