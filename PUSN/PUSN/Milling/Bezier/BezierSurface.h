#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>
#include <vector>
#include <math.h>
#include <string>
#include "../../StringHelper.h"

using namespace DirectX;
using namespace std;

class BezierSurfaceC0
{
public:
	vector<vector<XMFLOAT3>> _controlVertices;

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

		_controlVertices = vector < vector<XMFLOAT3>>(w);
		for (int i = 0; i < h; i++)
		{
			_controlVertices[i] = vector<XMFLOAT3>(h);
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

		_controlVertices = vector < vector<XMFLOAT3>>(h);
		for (int i = 0; i < h; i++)
		{
			_controlVertices[i] = vector<XMFLOAT3>(w);
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

	XMFLOAT3 StringToPosition(string data)
	{
		vector<string> parts;
		StringHelper::Split(data, parts, ';');
		return XMFLOAT3(stof(parts[0]), stof(parts[1]), stof(parts[2]));
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

	XMFLOAT3 GetValue(int idxH, int idxW, float tu, float tv)
	{
		XMVECTOR point = { 0,0,0 };
		for (int h = 0; h < 4; h++)
		{
			for (int w = 0; w < 4; w++)
			{
				point += XMLoadFloat3(&_controlVertices[3 * idxH + h][3 * idxW + w]) * GetB(h, tu) * GetB(w, tv);
			}
		}

		XMFLOAT3 result;
		XMStoreFloat3(&result, point);

		return result;
	}
	XMFLOAT3 GetValueDivH(int idxH, int idxW, float tu, float tv)
	{
		XMVECTOR point = { 0,0,0 };
		for (int h = 0; h < 4; h++)
		{
			for (int w = 0; w < 4; w++)
			{
				point += XMLoadFloat3(&_controlVertices[3 * idxH + h][3 * idxW + w]) * GetBDrv(h, tu) * GetB(w, tv);
			}
		}

		XMFLOAT3 result;
		XMStoreFloat3(&result, point);

		return result;
	}
	XMFLOAT3 GetValueDivW(int idxH, int idxW, float tu, float tv)
	{
		XMVECTOR point = { 0,0,0 };
		for (int h = 0; h < 4; h++)
		{
			for (int w = 0; w < 4; w++)
			{
				point += XMLoadFloat3(&_controlVertices[3 * idxH + h][3 * idxW + w]) * GetB(h, tu) * GetBDrv(w, tv);
			}
		}

		XMFLOAT3 result;
		XMStoreFloat3(&result, point);

		return result;
	}

	XMFLOAT3 Evaluate(XMFLOAT2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floorf(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floorf(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		return GetValue(ph, pw, hh, ww);
	}
	XMFLOAT3 EvaluateDU(XMFLOAT2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floorf(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floorf(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		XMFLOAT3 div = GetValueDivH(ph, pw, hh, ww);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&div) * heightPatchCount);

		return result;
	}
	XMFLOAT3 EvaluateDV(XMFLOAT2 hw)
	{
		float h = hw.x;
		float w = hw.y;

		int phc = heightPatchCount;
		int ph = (int)floorf(h * phc);
		if (ph == phc)
			ph = phc - 1;
		float hh = h * phc - ph;

		int pwc = widthPatchCount;
		int pw = (int)floorf(w * pwc);
		if (pw == pwc)
			pw = pwc - 1;
		float ww = w * pwc - pw;

		XMFLOAT3 div = GetValueDivW(ph, pw, hh, ww);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&div) * widthPatchCount);

		return result;
	}
	XMFLOAT3 EvaluateNormal(XMFLOAT2 hw) {
		XMFLOAT3 du = EvaluateDU(hw);
		XMFLOAT3 dv = EvaluateDV(hw);

		XMVECTOR cross = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&du), XMLoadFloat3(&dv)));
		XMFLOAT3 result;
		XMStoreFloat3(&result, cross);

		return result;
	}


};
