#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <direct.h>
#include <iomanip>
#include <SimpleMath.h>
#include <memory>

#include "..\\Graphics\\Vertex.h"
#include "..\\Graphics\\VertexBuffer.h"
#include "..\\Graphics\\IndexBuffer.h"
#include "..\\Graphics\\ConstantBuffer.h"
#include "Model.h"
#include "..\\Graphics\\Shaders.h"
#include "MillingMaterial.h"
#include "Bezier/BezierSurface.h"
#include "Bezier/IntersectionCurve.h"
#include "../StringHelper.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class PathGenerator
{
public:

	Model elephant;
	MillingMaterial* material;
	vector<vector<float>> heightMap;
	float minZ;
	float safeZ;

	PathGenerator(MillingMaterial* _material);

	void SavePath(vector<Vector3> moves, string filePath);

	void GenerateFirstPath();
	void GenerateSecondPath();
	void GenerateThirdPath();
	void GenerateHeightMap();
	float GetZ(float cpx, float cpy, float toolRadius);

	void EnsureInit();
	vector<Vector3> GenerateFirstPathLayer(float minZ);
	vector<Vector3> GenerateFlatEnvelope();
	BezierSurfaceC0 GetPlane();
	vector<Vector3> GenerateUnrestrictedPath(BezierSurfaceC0* surface, Vector3 startingPoint);
	vector<Vector3> GenerateUnrestrictedCylinderPath(BezierSurfaceC0* surface, bool top, float toolRange = 5.0f);
	bool SegmentsIntersect(const Vector2& A, const Vector2& B, const Vector2& C, const Vector2& D, Vector2& out);
	void RemoveSelfIntersections(vector<Vector3>& path);
	vector<Vector3> GenerateFlatLayer();

	bool DuplicateFirst(vector<Vector3>& path);
	bool DuplicateLast(vector<Vector3>& path);
	void Reverse(vector<Vector3>& path);

	vector<Vector3> GenerateSurfaceIntersectionPaths();
	vector<Vector3> GenerateUnrestrictedPath(BezierSurfaceC0* surface1, BezierSurfaceC0* surface2, Vector3 startingPoint);
	vector<Vector3> GenerateSurfacePaths();
	vector<vector<Vector3>> AddParametrizationLine(BezierSurfaceC0* surface, bool wdir, bool notZero);

	void Append(vector<Vector3>& path, vector<vector<Vector3>>& toAppend);
	void Append(vector<Vector3>& path, vector<Vector3>& toAppend);

	void RemoveEmpty(vector<vector<Vector3>>& paths);
	void AddSafe(vector<Vector3>& path);
	void AddSafe(vector<Vector3>& path, float safeH);
	void AddSafeStart(vector<Vector3>& path);
	void AddSafeStart(vector<Vector3>& path, float safeH);
	void AddSafeEnd(vector<Vector3>& path);
	void AddSafeEnd(vector<Vector3>& path, float safeH);

	void TrimEnd(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer);
	void TrimStart(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer);
	void TrimEndLast(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer);
	void TrimStartLast(vector<vector<Vector3>>& paths, vector<Vector3>& trimmer);
	void Finalize(vector<vector<Vector3>>& path, float interHeight);
	void TrimCenter(vector<vector<Vector3>>& paths, vector<Vector3>& trimmerFrom, vector<Vector3>& trimmerTo);
	void AddOuterSafe(vector<vector<Vector3>>& path);
	void AddInnerSafe(vector<vector<Vector3>>& path, float height);
	void TrimEnd(vector<Vector3>& path, vector<Vector3>& trimmer);
	void TrimStart(vector<Vector3>& path, vector<Vector3>& trimmer);
	void TrimEndLast(vector<Vector3>& path, vector<Vector3>& trimmer);
	void TrimStartLast(vector<Vector3>& path, vector<Vector3>& trimmer);
	void TrimCenter(vector<Vector3>& path, vector<Vector3>& pathOut, vector<Vector3>& trimmerFrom, vector<Vector3>& trimmerTo);
	int FindIntersection(vector<Vector3>& path, vector<Vector3>& trimmer);
	int FindIntersectionLast(vector<Vector3>& path, vector<Vector3>& trimmer);
	bool FindIntersection(vector<Vector3>& path1, vector<Vector3>& path2, int& idx1, int& idx2);
	bool FindIntersectionLast(vector<Vector3>& path1, vector<Vector3>& path2, int& idx1, int& idx2);

};

