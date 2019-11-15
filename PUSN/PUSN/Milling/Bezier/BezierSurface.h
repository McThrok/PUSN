#pragma once

#include <d3d11.h>
#include <math.h>
#include <vector>
#include <math.h>
#include <string>
#include "../../StringHelper.h"
#include <SimpleMath.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class BezierSurfaceC0
{
public:
	vector<vector<Vector3>> _controlVertices;

	static int count;
	int id;
	int widthPatchCount;
	int heightPatchCount;
	bool WrappedV;

	int GetId() { return id; };
	bool IsWrappedU() { return false; }
	bool IsWrappedV() { return WrappedV; }

	BezierSurfaceC0(int ph, int pw, bool cylinder = false)
	{
		id = count;
		count++;

		WrappedV = cylinder;
		heightPatchCount = ph;
		widthPatchCount = pw;
		int h = GetHeightVertexCount();
		int w = GetWidthVertexCount();

		_controlVertices = vector < vector<Vector3>>(h);
		for (int i = 0; i < h; i++)
		{
			_controlVertices[i] = vector<Vector3>(w);
			for (int j = 0; j < w; j++) {
				if (cylinder && j == w - 1)
					_controlVertices[i][j] = _controlVertices[i][0];
				else
					_controlVertices[i][j] = { (float)i,(float)j,0 };
			}
		}
	}
	BezierSurfaceC0(string data, bool cylinder = false)
	{
		id = count;
		count++;

		vector<string> parts;
		StringHelper::Split(data, parts);

		WrappedV = cylinder;
		heightPatchCount = stoi(parts[1]);
		widthPatchCount = stoi(parts[2]);
		int h = GetHeightVertexCount();
		int w = GetWidthVertexCount();

		_controlVertices = vector < vector<Vector3>>(h);
		for (int i = 0; i < h; i++)
		{
			_controlVertices[i] = vector<Vector3>(w);
			for (int j = 0; j < w; j++) {
				if (cylinder) {
					if (j == w - 1)
						_controlVertices[i][j] = _controlVertices[i][0];
					else
						_controlVertices[i][j] = StringToPosition(parts[i * (w - 1) + j + 3]);
				}
				else {
					_controlVertices[i][j] = StringToPosition(parts[i * w + j + 3]);
				}
			}
		}

	}

	Vector3 StringToPosition(string data)
	{
		vector<string> parts;
		StringHelper::Split(data, parts, ';');
		return Vector3(stof(parts[0]), stof(parts[1]), stof(parts[2]));
	}

	Vector3& GetVert(int w, int h)
	{
		return _controlVertices[h][w];
	}

	int GetWidthVertexCount()
	{
		return 3 * widthPatchCount + 1;
	}
	int GetHeightVertexCount()
	{
		return 3 * heightPatchCount + 1;
	}

	float GetB(int i, float t)
	{
		float c = 1.0f - t;

		if (i == 0)
			return c * c * c;
		if (i == 1)
			return 3 * t * c * c;
		if (i == 2)
			return 3 * t * t * c;
		if (i == 3)
			return t * t * t;

		return 0;
	}
	float GetBDrv(int i, float t)
	{
		float c = 1.0f - t;

		if (i == 0)
			return -3 * c * c;
		if (i == 1)
			return 3 * (-2 * t + c) * c;
		if (i == 2)
			return 3 * t * (2 - 3 * t);
		if (i == 3)
			return 3 * t * t;

		return 0;
	}

	Vector3 GetValue(int idxH, int idxW, float tu, float tv)
	{
		Vector3 point = Vector3::Zero;
		for (int h = 0; h < 4; h++)
			for (int w = 0; w < 4; w++)
				point += _controlVertices[3 * idxH + h][3 * idxW + w] * GetB(h, tu) * GetB(w, tv);

		return point;
	}
	Vector3 GetValueDivH(int idxH, int idxW, float tu, float tv)
	{
		Vector3 point = Vector3::Zero;
		for (int h = 0; h < 4; h++)
			for (int w = 0; w < 4; w++)
				point += _controlVertices[3 * idxH + h][3 * idxW + w] * GetBDrv(h, tu) * GetB(w, tv);

		return point;
	}
	Vector3 GetValueDivW(int idxH, int idxW, float tu, float tv)
	{
		Vector3 point = Vector3::Zero;
		for (int h = 0; h < 4; h++)
			for (int w = 0; w < 4; w++)
				point += _controlVertices[3 * idxH + h][3 * idxW + w] * GetB(h, tu) * GetBDrv(w, tv);

		return point;
	}

	Vector3 Evaluate(Vector2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floor(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floor(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		return GetValue(ph, pw, hh, ww);
	}
	Vector3 EvaluateDU(Vector2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floor(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floor(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		return GetValueDivH(ph, pw, hh, ww) * heightPatchCount;
	}
	Vector3 EvaluateDV(Vector2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floor(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floor(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		return GetValueDivW(ph, pw, hh, ww) * widthPatchCount;
	}

	Vector3 EvaluateNormal(Vector2 hw) {
		Vector3 du = EvaluateDU(hw);
		Vector3 dv = EvaluateDV(hw);

		Vector3 result = dv.Cross(du);
		result.Normalize();

		return result;
	}

	//Vector3 GetValue(vector<vector<Vector3>> verts, int idxH, int idxW, float tu, float tv)
	//{
	//	Vector3 point = Vector3::Zero;
	//	for (int h = 0; h < 4; h++)
	//	{
	//		for (int w = 0; w < 4; w++)
	//		{
	//			point += verts[idxH + h][idxW + w] * GetB(h, tu) * GetB(w, tv);
	//		}
	//	}

	//	return point;
	//}
	//Vector3 GetValueDivH(vector<vector<Vector3>> verts, int idxH, int idxW, float tu, float tv)
	//{
	//	Vector3 point = Vector3::Zero;
	//	for (int h = 0; h < 4; h++)
	//	{
	//		for (int w = 0; w < 4; w++)
	//		{
	//			point += verts[idxH + h][idxW + w] * GetBDrv(h, tu) * GetB(w, tv);
	//		}
	//	}

	//	return point;
	//}
	//Vector3 GetValueDivW(vector<vector<Vector3>> verts, int idxH, int idxW, float tu, float tv)
	//{
	//	Vector3 point = Vector3::Zero;
	//	for (int h = 0; h < 4; h++)
	//	{
	//		for (int w = 0; w < 4; w++)
	//		{
	//			point += verts[idxH + h][idxW + w] * GetB(h, tu) * GetBDrv(w, tv);
	//		}
	//	}

	//	return point;
	//}

	//vector<vector<Vector3>> GetPatchVerts(int h, int w)
	//{
	//	vector<vector<Vector3>>verts(4);
	//	for (int i = 0; i < 4; i++)
	//	{
	//		verts.push_back(vector<Vector3>(4));
	//		for (int j = 0; j < 4; j++)
	//		{
	//			verts[i].push_back(_controlVertices[3 * h + i][3 * w + j]);
	//		}
	//	}

	//	return verts;
	//}
	//Vector3 Evaluate(Vector2 hw)
	//{
	//	float h = hw.x;
	//	float w = hw.y;

	//	int phc = heightPatchCount;
	//	int ph = (int)floor(h * phc);
	//	if (ph == phc)
	//		ph = phc - 1;
	//	float hh = h * phc - ph;

	//	int pwc = widthPatchCount;
	//	int pw = (int)floor(w * pwc);
	//	if (pw == pwc)
	//		pw = pwc - 1;
	//	float ww = w * pwc - pw;

	//	return GetValue(GetPatchVerts(ph, pw), 0, 0, hh, ww);
	//}
	//Vector3 EvaluateDU(Vector2 hw)
	//{
	//	float h = hw.x;
	//	float w = hw.y;

	//	int phc = heightPatchCount;
	//	int ph = (int)floor(h * phc);
	//	if (ph == phc)
	//		ph = phc - 1;
	//	float hh = h * phc - ph;

	//	int pwc = widthPatchCount;
	//	int pw = (int)floor(w * pwc);
	//	if (pw == pwc)
	//		pw = pwc - 1;
	//	float ww = w * pwc - pw;

	//	return GetValueDivH(GetPatchVerts(ph, pw), 0, 0, hh, ww) * heightPatchCount;
	//}
	//Vector3 EvaluateDV(Vector2 hw)
	//{
	//	float h = hw.x;
	//	float w = hw.y;

	//	int phc = heightPatchCount;
	//	int ph = (int)floor(h * phc);
	//	if (ph == phc)
	//		ph = phc - 1;
	//	float hh = h * phc - ph;

	//	int pwc = widthPatchCount;
	//	int pw = (int)floor(w * pwc);
	//	if (pw == pwc)
	//		pw = pwc - 1;
	//	float ww = w * pwc - pw;

	//	return GetValueDivW(GetPatchVerts(ph, pw), 0, 0, hh, ww) * widthPatchCount;
	//}
};
