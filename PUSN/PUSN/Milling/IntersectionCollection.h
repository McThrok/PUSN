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

class IntersectionCollection
{
public:
	ModelVersion* model0;
	ModelVersion* model8;
	float minZ;

	vector<Vector3> GetTorsoLegFront8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetLegFront(), model8->GetTorso(), Vector3(20, -20, minZ - 20));
		static bool done = false;
		if (!done) {
			result.erase(result.begin(), result.begin() + 25);
			done = true;
		}
		return result;
	}
	vector<Vector3> GetTorsoBoxRight8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetTorso(), model8->GetBox(), Vector3(-5, 20, minZ + 10));
		return result;
	}
	vector<Vector3> GetTorsoBoxLeft8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetTorso(), model8->GetBox(), Vector3(-15, 20, minZ + 10));
		return result;
	}
	vector<Vector3> GetTorsoLegBack8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetLegBack(), model8->GetTorso(), Vector3(-50, -20, minZ - 10));
		return result;
	}
	vector<Vector3> GetTorsoHead8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetHead(), model8->GetTorso(), Vector3(70, 0, minZ + 10));
		return result;
	}
	vector<Vector3> GetHeadEarLeft8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetHead(), model8->GetRightEar(), Vector3(20, 20, minZ + 10));
		return result;
	}
	vector<Vector3> GetHeadEarRight8()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetHead(), model8->GetRightEar(), Vector3(50, 20, minZ + 10));
		return result;
	}
	vector<Vector3> GetTorsoTail()
	{
		static auto result = GenerateUnrestrictedPath(model8->GetTorso(), model8->GetTail(), Vector3(50, -20, minZ - 10));
		return result;
	}

private:
	vector<Vector3> GenerateUnrestrictedPath(BezierSurfaceC0* surface1, BezierSurfaceC0* surface2, Vector3 startingPoint)
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

				if (position.z > minZ)
					if (result.size() == 0 || Vector3::Distance(result[result.size() - 1], position) > filterDist)
						result.push_back(position);
			}
			delete curve;
		}

		return result;
	}
};