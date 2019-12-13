#pragma once

#include <vector>
#include <DirectXMath.h>
#include <math.h>
#include "BezierSurface.h"
#include <SimpleMath.h>
#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct UpdStruct
{
	BezierSurfaceC0* Obj;
	XMFLOAT2 UV;
	XMFLOAT2 UVNew;
	bool Backed;
};

struct UpdateUVStruct
{
	UpdateUVStruct(XMFLOAT2 _UV, bool _End, bool _Back) :UV(_UV), End(_End), Back(_Back) {}

	XMFLOAT2 UV;
	bool End;
	bool Back;
};

class IntersectionCurve
{
public:
	static int _count;

	static float _gradientEps;
	static float _startGradientAlpha;

	static float _newtonStartAlpha;
	static float _finalEpsilon;
	static float _alphaEpsilon;


	vector<Vector3> Verts;
	vector<Vector2> _uv0;
	vector<Vector2> _uv1;

	IntersectionCurve() {}
	IntersectionCurve(vector<Vector3> verts, vector<Vector2> uv0, vector<Vector2> uv1)
	{
		Verts = verts;
		_uv0 = uv0;
		_uv1 = uv1;
	}

	static IntersectionCurve* FindIntersectionCurve(vector<BezierSurfaceC0*> objs, Vector3 cursorPos, float precision)
	{
		float maxDist = 100000;
		Vector2 p0 = Vector2::Zero;
		Vector2 p1 = Vector2::Zero;


		vector<Vector3> qwe;
		int divCount = 13;//qwe
		//int divCount = 5;
		bool oneObject = objs[0]->GetId() == objs[1]->GetId();
		float eps = 0.5f;


		for (int i = 0; i < divCount; i++)
			for (int j = 0; j < divCount; j++)
				for (int k = 0; k < divCount; k++)
					for (int m = 0; m < divCount; m++)
					{
						float ii = 1.0f * i / (divCount - 1);
						float jj = 1.0f * j / (divCount - 1);
						float kk = 1.0f * k / (divCount - 1);
						float mm = 1.0f * m / (divCount - 1);

						Vector3 ev1 = objs[0]->Evaluate(Vector2(ii, jj));
						Vector3 ev2 = objs[1]->Evaluate(Vector2(kk, mm));

						//qwe.push_back(ev1);
						float dist = Vector3::Distance(ev1, cursorPos) + Vector3::Distance(ev2, cursorPos);
						if (dist < maxDist && (!oneObject || (eps < abs(ii - kk) && eps < abs(jj - mm))))
						{
							p0 = Vector2(ii, jj);
							p1 = Vector2(kk, mm);
							maxDist = dist;
						}
					}

		//return new IntersectionCurve(qwe, {}, {});
		return Gradient(objs[0], objs[1], p0, p1, precision);
	}

	static IntersectionCurve* Gradient(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 value0, Vector2 value1, float precision)
	{
		Vector3 p0 = obj0->Evaluate(value0);
		Vector3 p1 = obj1->Evaluate(value1);

		int i = 0;
		float currAlpha = _startGradientAlpha/10;
		float dist = Vector3::Distance(p1, p0);

		vector<Vector3> q0;//qwe
		vector<Vector3> q1;//qwe
		q0.push_back(p0);
		q1.push_back(p1);

		while (dist > _gradientEps)
		{
			if (++i > 10000) {
				return nullptr;
			}

			try
			{
				vector<Vector2> grads = GetGradient(obj0, obj1, value0, value1);
				value0 -= currAlpha * grads[0];
				value1 -= currAlpha * grads[1];

				value0.x = min(1.0f, max(0.0f, value0.x));
				value0.y = min(1.0f, max(0.0f, value0.y));
				value1.x = min(1.0f, max(0.0f, value1.x));
				value1.y = min(1.0f, max(0.0f, value1.y));

				Vector3 pNew0 = obj0->Evaluate(value0);
				Vector3 pNew1 = obj1->Evaluate(value1);
				q0.push_back(pNew0);
				q1.push_back(pNew1);

				float newDist = Vector3::Distance(pNew0, pNew1);
				if (newDist > dist)
				{
					currAlpha /= 2;
					currAlpha = max(currAlpha, 0.0001f);
				}
				else
				{
					currAlpha *= 2.0f;
					dist = newDist;
					p0 = pNew0;
					p1 = pNew1;

				}
			}
			catch (...)
			{
				return nullptr;
			}
		}

		return MyFriendNewton(obj0, obj1, value0, value1, precision);
	}
	static vector<Vector2> GetGradient(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 point0, Vector2 point1)
	{
		Vector3 eval0 = obj0->Evaluate(point0);
		Vector3 eval1 = obj1->Evaluate(point1);

		Vector3 diff = eval0 - eval1;

		Vector3 eval0u = obj0->EvaluateDU(point0);
		Vector3 eval0v = obj0->EvaluateDV(point0);
		eval0u.Normalize();
		eval0v.Normalize();

		Vector3 eval1u = obj1->EvaluateDU(point1);
		Vector3 eval1v = obj1->EvaluateDV(point1);
		eval1u.Normalize();
		eval1v.Normalize();

		Vector2 grad0 = Vector2(diff.Dot(eval0u), diff.Dot(eval0v));
		Vector2 grad1 = Vector2((-diff).Dot(eval1u), (-diff).Dot(eval1v));

		return vector<Vector2> { grad0, grad1 };
	}
	static IntersectionCurve* MyFriendNewton(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 uv0, Vector2 uv1, float precision)
	{
		float newtonAlpha = _newtonStartAlpha;

		Vector2 uvStart0 = uv0;
		Vector2 uvStart1 = uv1;
		Vector2 uvPrev0 = uvStart0;
		Vector2 uvPrev1 = uvStart1;

		float backed = false;
		float finished = false;

		Vector3 pStart = obj0->Evaluate(uvStart0);
		int countForCylinder = 0;
		int loops = 0;

		vector<Vector3> pointsList;
		vector<Vector2> uvList0;
		vector<Vector2> uvList1;

		pointsList.push_back(obj0->Evaluate(uv0));
		uvList0.push_back(uv0);
		uvList1.push_back(uv1);

		while (!finished)
		{
			float currAlpha = newtonAlpha;
			int innerLoops = 0;
			while (true)
			{
				Vector4 betterPoint = GetNewtonIterationPoint(obj0, obj1, uvPrev0, uvPrev1, uv0, uv1, currAlpha);

				Vector2 uvDiff0 = Vector2(betterPoint.x, betterPoint.y);
				Vector2 ufDivv1 = Vector2(betterPoint.z, betterPoint.w);

				UpdateUVStruct upd0 = UpdateUV(obj0, uv0, uvDiff0, backed);
				UpdateUVStruct upd1 = UpdateUV(obj1, uv1, ufDivv1, backed);

				uv0 = upd0.UV;
				uv1 = upd1.UV;

				if (upd0.End || upd1.End)
				{
					finished = true;
					break;
				}

				if (upd0.Back || upd1.Back)
				{
					reverse(pointsList.begin(), pointsList.end());
					reverse(uvList0.begin(), uvList0.end());
					reverse(uvList1.begin(), uvList1.end());

					uv0 = uvStart0;
					uv1 = uvStart1;

					uvPrev0 = uvStart0;
					uvPrev1 = uvStart1;

					newtonAlpha = -_newtonStartAlpha;

					countForCylinder = 5;
					backed = true;
					currAlpha = newtonAlpha;
					break;
				}

				Vector3 ev0 = obj0->Evaluate(uv0);
				Vector3 ev1 = obj1->Evaluate(uv1);
				float dst = Vector3::Distance(ev0, ev1);
				if (precision > dst)
					break;

				if (++innerLoops > 30)
					return nullptr;
			}

			uvPrev0 = uv0;
			uvPrev1 = uv1;

			Vector3 p1 = obj0->Evaluate(uv0);
			Vector3 p2 = obj1->Evaluate(uv1);
			float dist = Vector3::Distance(p2, p1);
			if (_alphaEpsilon < Vector3::Distance(p2, p1))
			{
				currAlpha /= 2;
			}

			pointsList.push_back(obj0->Evaluate(uv0));
			uvList0.push_back(uv0);
			uvList1.push_back(uv1);

			if (loops > 2000 || _finalEpsilon > Vector3::Distance(pStart, p1) && countForCylinder > 10)
			{
				break;
			}

			countForCylinder++;
			loops++;
		}

		return new IntersectionCurve(pointsList, uvList0, uvList1);
	}

	static Vector4 GetNewtonIterationPoint(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 uv0, Vector2 uv1, Vector2 uvNew0, Vector2 uvNew1, float alpha)
	{
		Matrix mat = GetJacobi(obj0, obj1, uv0, uv1, uvNew0, uvNew1);
		Vector4 vec = GetF(obj0, obj1, uv0, uv1, uvNew0, uvNew1, alpha);

		return Vector4::Transform(vec, mat.Transpose());
	}
	static Matrix GetJacobi(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 uv0, Vector2 uv1, Vector2 uvNew0, Vector2 uvNew1)
	{
		Vector3 dU0 = obj0->EvaluateDU(uv0);
		Vector3 dV0 = obj0->EvaluateDV(uv0);
		Vector3 dU1 = obj1->EvaluateDU(uv1);
		Vector3 dV1 = obj1->EvaluateDV(uv1);

		Vector3 normalT = GetTNormal(dU0, dU1, dV0, dV1);
		dU0 = obj0->EvaluateDU(uvNew0);
		dV0 = obj0->EvaluateDV(uvNew0);
		dU1 = -obj1->EvaluateDU(uvNew1);
		dV1 = -obj1->EvaluateDV(uvNew1);

		float dot1 = dU0.Dot(normalT);
		float dot2 = dV0.Dot(normalT);

		//rows vs columns
		Matrix jacobiMatrix = Matrix(
			dU0.x, dV0.x, dU1.x, dV1.x,
			dU0.y, dV0.y, dU1.y, dV1.y,
			dU0.z, dV0.z, dU1.z, dV1.z,
			dot1, dot2, 0, 0);

		return jacobiMatrix.Invert();
	}
	static Vector4 GetF(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, Vector2 uv0, Vector2 uv1, Vector2 uvNew0, Vector2 uvNew1, float alpha)
	{
		Vector3 P0 = obj0->Evaluate(uv0);
		Vector3 Q = obj1->Evaluate(uvNew1);
		Vector3 P1 = obj0->Evaluate(uvNew0);

		Vector3 dU0 = obj0->EvaluateDU(uv0);
		Vector3 dV0 = obj0->EvaluateDV(uv0);
		Vector3 dU1 = obj1->EvaluateDU(uv1);
		Vector3 dV1 = obj1->EvaluateDV(uv1);

		Vector3 normalT = GetTNormal(dU0, dU1, dV0, dV1);
		//float d = alpha;
		float d = alpha * 10;

		Vector3 tmp = P1 - Q;
		return Vector4(tmp.x, tmp.y, tmp.z, (P1 - P0).Dot(normalT) - d);
	}
	static Vector3 GetTNormal(Vector3 du0, Vector3 du1, Vector3 dv0, Vector3 dv1)
	{
		Vector3 np = du0.Cross(dv0);
		Vector3 nq = du1.Cross(dv1);
		np.Normalize();
		nq.Normalize();

		Vector3 normalT = np.Cross(nq);
		normalT.Normalize();
		return normalT;
	}

	static UpdateUVStruct UpdateUV(BezierSurfaceC0* obj, Vector2 uv, Vector2 uvDiff, bool backed)
	{
		bool backThisTime = false;
		bool end = false;

		float _uNew = uv.x - uvDiff.x;
		float _vNew = uv.y - uvDiff.y;


		if (_uNew < 0)
		{
			if (obj->IsWrappedU())
			{
				_uNew = 1;
			}
			else
			{
				_uNew = 0;
				if (backed)
				{
					end = true;
				}
				else
				{
					backThisTime = true;
				}
			}
		}
		else if (_uNew > 1)
		{
			if (obj->IsWrappedU())
			{
				_uNew = 0;
			}
			else
			{
				_uNew = 1;
				if (backed)
				{
					end = true;
				}
				else
				{
					backThisTime = true;
				}
			}
		}

		if (_vNew > 1)
		{
			if (obj->IsWrappedV())
			{
				_vNew = 0;
			}
			else
			{
				_vNew = 1;
				if (backed)
				{
					end = true;
				}
				else
				{
					backThisTime = true;
				}
			}
		}
		else if (_vNew < 0)
		{
			if (obj->IsWrappedV())
			{
				_vNew = 1;
			}
			else
			{
				_vNew = 0;
				if (backed)
				{
					end = true;
				}
				else
				{
					backThisTime = true;
				}
			}
		}

		return UpdateUVStruct(Vector2(_uNew, _vNew), end, backThisTime);
	}
};
