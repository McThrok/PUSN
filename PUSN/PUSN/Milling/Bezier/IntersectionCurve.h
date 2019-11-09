#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>
#include "BezierSurface.h"

using namespace DirectX;
using namespace std;

struct UpdStruct
{
	BezierSurfaceC0 * Obj;
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

//
//class IntersectionCurve
//{
//	//private static int _count = 0;
//
//	//private static float _gradientEps = 0.0001f;
//	//private static float _startGradientAlpha = 0.01f;
//
//	//public static float _newtonStartAlpha = 0.002f;
//	//public static float _finalEpsilon = 0.01f;
//	//public static float _alphaEpsilon = 0.001f;
//
//	
//	public List<XMFLOAT3> Verts{ get; }
//	private readonly List<XMFLOAT3> _uv0;
//	private readonly List<XMFLOAT3> _uv1;
//
//	public IntersectionCurve(List<XMFLOAT3> verts, List<XMFLOAT2> uv0, List<XMFLOAT2> uv1) : this()
//	{
//		Verts = verts;
//		_uv0 = uv0.Select(v = > new XMFLOAT3(v, 0)).ToList();
//		_uv1 = uv1.Select(v = > new XMFLOAT3(v, 0)).ToList();
//	}
//
//	public static IntersectionCurve FindIntersectionCurve(List<BezierSurfaceC0> objs, XMFLOAT3 cursorPos, float precision)
//	{
//		float maxDist = float.MaxValue;
//		XMFLOAT2 p0 = XMFLOAT2.zero;
//		XMFLOAT2 p1 = XMFLOAT2.zero;
//
//		int divCount = 13;
//		bool oneObject = objs[0].Id == objs[1].Id;
//		float eps = 0.5f;
//
//		for (int i = 0; i < divCount; i++)
//			for (int j = 0; j < divCount; j++)
//				for (int k = 0; k < divCount; k++)
//					for (int m = 0; m < divCount; m++)
//					{
//						float ii = 1f * i / (divCount - 1);
//						float jj = 1f * j / (divCount - 1);
//						float kk = 1f * k / (divCount - 1);
//						float mm = 1f * m / (divCount - 1);
//
//						var ev1 = objs[0].Evaluate(new XMFLOAT2(ii, jj));
//						var ev2 = objs[1].Evaluate(new XMFLOAT2(kk, mm));
//						var dist = XMFLOAT3.Distance(ev1, cursorPos) + XMFLOAT3.Distance(ev2, cursorPos);
//						if (dist < maxDist && (!oneObject || (eps < Math.Abs(ii - kk) && eps < Math.Abs(jj - mm))))
//						{
//							p0 = new XMFLOAT2(ii, jj);
//							p1 = new XMFLOAT2(kk, mm);
//							maxDist = dist;
//						}
//					}
//
//		return Gradient(objs[0], objs[1], p0, p1, precision);
//	}
//
//	private static IntersectionCurve Gradient(BezierSurfaceC0 obj0, BezierSurfaceC0 obj1, XMFLOAT2 value0, XMFLOAT2 value1, float precision)
//	{
//		var p0 = obj0.Evaluate(value0);
//		var p1 = obj1.Evaluate(value1);
//
//		var i = 0;
//		var currAlpha = _startGradientAlpha;
//		var dist = XMFLOAT3.Distance(p1, p0);
//		while (dist > _gradientEps)
//		{
//			if (++i > 10000)
//				return null;
//
//			try
//			{
//				var grads = GetGradient(obj0, obj1, value0, value1);
//				value0 -= currAlpha * grads[0];
//				value1 -= currAlpha * grads[1];
//
//				value0.x = Math.Min(1, Math.Max(0, value0.x));
//				value0.y = Math.Min(1, Math.Max(0, value0.y));
//				value1.x = Math.Min(1, Math.Max(0, value1.x));
//				value1.y = Math.Min(1, Math.Max(0, value1.y));
//
//				var pNew0 = obj0.Evaluate(value0);
//				var pNew1 = obj1.Evaluate(value1);
//
//				var newDist = XMFLOAT3.Distance(pNew0, pNew1);
//				if (newDist > dist)
//				{
//					currAlpha /= 2;
//					currAlpha = Math.Max(currAlpha, 0.0001f);
//				}
//				else
//				{
//					currAlpha *= 2;
//					dist = newDist;
//					p0 = pNew0;
//					p1 = pNew1;
//				}
//			}
//			catch (Exception e)
//			{
//				return null;
//			}
//		}
//
//		return MyFriendNewton(obj0, obj1, value0, value1, precision);
//	}
//	private static List<XMFLOAT2> GetGradient(BezierSurfaceC0 obj0, BezierSurfaceC0 obj1, XMFLOAT2 point0, XMFLOAT2 point1)
//	{
//		var eval0 = obj0.Evaluate(point0);
//		var eval1 = obj1.Evaluate(point1);
//
//		var diff = eval0 - eval1;
//
//		var eval0u = obj0.EvaluateDU(point0).Normalized();
//		var eval0v = obj0.EvaluateDV(point0).Normalized();
//
//		var eval1u = obj1.EvaluateDU(point1).Normalized();
//		var eval1v = obj1.EvaluateDV(point1).Normalized();
//
//		var grad0 = new XMFLOAT2(XMFLOAT3.Dot(diff, eval0u), XMFLOAT3.Dot(diff, eval0v));
//		var grad1 = new XMFLOAT2(XMFLOAT3.Dot(-diff, eval1u), XMFLOAT3.Dot(-diff, eval1v));
//
//		return new List<XMFLOAT2>() { grad0, grad1 };
//	}
//	private static IntersectionCurve MyFriendNewton(BezierSurfaceC0 obj0, BezierSurfaceC0 obj1, XMFLOAT2 uv0, XMFLOAT2 uv1, float precision)
//	{
//		var newtonAlpha = _newtonStartAlpha;
//
//		var uvStart0 = uv0;
//		var uvStart1 = uv1;
//		var uvPrev0 = uvStart0;
//		var uvPrev1 = uvStart1;
//
//		var backed = false;
//		var finished = false;
//
//		var pStart = obj0.Evaluate(uvStart0);
//		var countForCylinder = 0;
//		var loops = 0;
//
//		var pointsList = new List<XMFLOAT3>();
//		var uvList0 = new List<XMFLOAT2>();
//		var uvList1 = new List<XMFLOAT2>();
//
//		pointsList.Add(obj0.Evaluate(uv0));
//		uvList0.Add(uv0);
//		uvList1.Add(uv1);
//
//		while (!finished)
//		{
//			var currAlpha = newtonAlpha;
//			int innerLoops = 0;
//			while (true)
//			{
//				var betterPoint = GetNewtonIterationPoint(obj0, obj1, uvPrev0, uvPrev1, uv0, uv1, currAlpha);
//
//				var uvDiff0 = new XMFLOAT2(betterPoint.x, betterPoint.y);
//				var ufDivv1 = new XMFLOAT2(betterPoint.z, betterPoint.w);
//
//				var upd0 = UpdateUV(obj0, uv0, uvDiff0, backed);
//				var upd1 = UpdateUV(obj1, uv1, ufDivv1, backed);
//				uv0 = upd0.UV;
//				uv1 = upd1.UV;
//
//				if (upd0.End || upd1.End)
//				{
//					finished = true;
//					break;
//				}
//
//				if (upd0.Back || upd1.Back)
//				{
//					pointsList.Reverse();
//					uvList0.Reverse();
//					uvList1.Reverse();
//
//					uv0 = uvStart0;
//					uv1 = uvStart1;
//
//					uvPrev0 = uvStart0;
//					uvPrev1 = uvStart1;
//
//					newtonAlpha = -_newtonStartAlpha;
//
//					countForCylinder = 5;
//					backed = true;
//					currAlpha = newtonAlpha;
//					break;
//				}
//
//				var ev0 = obj0.Evaluate(uv0);
//				var ev1 = obj1.Evaluate(uv1);
//				var dst = XMFLOAT3.Distance(ev0, ev1);
//				if (precision > dst)
//					break;
//
//				if (++innerLoops > 30)
//					return null;
//			}
//
//			uvPrev0 = uv0;
//			uvPrev1 = uv1;
//
//			var p1 = obj0.Evaluate(uv0);
//			var p2 = obj1.Evaluate(uv1);
//			var dist = XMFLOAT3.Distance(p2, p1);
//			if (_alphaEpsilon < XMFLOAT3.Distance(p2, p1))
//			{
//				currAlpha /= 2;
//			}
//
//			pointsList.Add(obj0.Evaluate(uv0));
//			uvList0.Add(uv0);
//			uvList1.Add(uv1);
//
//			if (loops > 1000 || _finalEpsilon > XMFLOAT3.Distance(pStart, p1) && countForCylinder > 10)
//			{
//				break;
//			}
//
//			countForCylinder++;
//			loops++;
//		}
//
//		return new IntersectionCurve(pointsList, uvList0, uvList1);
//	}
//
//	private static Vector4 GetNewtonIterationPoint(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, XMFLOAT2 uv0, XMFLOAT2 uv1, XMFLOAT2 uvNew0, XMFLOAT2 uvNew1, float alpha)
//	{
//		var mat = GetJacobi(obj0, obj1, uv0, uv1, uvNew0, uvNew1);
//		var vec = GetF(obj0, obj1, uv0, uv1, uvNew0, uvNew1, alpha);
//		return vec.Multiply(mat);
//	}
//	public static Matrix4x4 GetJacobi(BezierSurfaceC0* obj0, BezierSurfaceC0* obj1, XMFLOAT2 uv0, XMFLOAT2 uv1, XMFLOAT2 uvNew0, XMFLOAT2 uvNew1)
//	{
//		XMFLOAT3 dU0 = obj0->EvaluateDU(uv0);
//		XMFLOAT3 dV0 = obj0->EvaluateDV(uv0);
//		XMFLOAT3 dU1 = obj1->EvaluateDU(uv1);
//		XMFLOAT3 dV1 = obj1->EvaluateDV(uv1);
//
//		XMFLOAT3 normalT = GetTNormal(dU0, dU1, dV0, dV1);
//		dU0 = obj0->EvaluateDU(uvNew0);
//		dV0 = obj0->EvaluateDV(uvNew0);
//		dU1 = -obj1->EvaluateDU(uvNew1);
//		dV1 = -obj1->EvaluateDV(uvNew1);
//
//		var dot1 = XMFLOAT3.Dot(dU0, normalT);
//		var dot2 = XMFLOAT3.Dot(dV0, normalT);
//
//		var jacobiMatrix = new Matrix4x4(
//			dU0.x, dV0.x, dU1.x, dV1.x,
//			dU0.y, dV0.y, dU1.y, dV1.y,
//			dU0.z, dV0.z, dU1.z, dV1.z,
//			dot1, dot2, 0, 0);
//
//		Matrix4x4.Invert(jacobiMatrix, out Matrix4x4 inv);
//		return inv;
//	}
//	public static Vector4 GetF(BezierSurfaceC0 obj0, BezierSurfaceC0 obj1, XMFLOAT2 uv0, XMFLOAT2 uv1, XMFLOAT2 uvNew0, XMFLOAT2 uvNew1, float alpha)
//	{
//		XMFLOAT3 P0 = obj0.Evaluate(uv0);
//		XMFLOAT3 Q = obj1.Evaluate(uvNew1);
//		XMFLOAT3 P1 = obj0.Evaluate(uvNew0);
//
//		XMFLOAT3 dU0 = obj0.EvaluateDU(uv0);
//		XMFLOAT3 dV0 = obj0.EvaluateDV(uv0);
//		XMFLOAT3 dU1 = obj1.EvaluateDU(uv1);
//		XMFLOAT3 dV1 = obj1.EvaluateDV(uv1);
//
//		XMFLOAT3 normalT = GetTNormal(dU0, dU1, dV0, dV1);
//		float d = alpha * 10;
//
//		return new XMFLOAT4(P1 - Q, XMFLOAT3.Dot(P1 - P0, normalT) - d);
//	}
//	public static XMFLOAT3 GetTNormal(XMFLOAT3 du0, XMFLOAT3 du1, XMFLOAT3 dv0, XMFLOAT3 dv1)
//	{
//
//		XMVECTOR np = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&du0), XMLoadFloat3(&dv0)));
//		XMVECTOR nq = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&du1), XMLoadFloat3(&dv1)));
//		XMFLOAT3 normalT;
//		XMStoreFloat3(&normalT, XMVector3Normalize(XMVector3Cross(np, nq)));
//
//		return normalT;
//	}
//
//	private static UpdateUVStruct UpdateUV(BezierSurfaceC0* obj, XMFLOAT2 uv, XMFLOAT2 uvDiff, bool backed)
//	{
//		bool backThisTime = false;
//		bool end = false;
//
//		float _uNew = uv.x - uvDiff.x;
//		float _vNew = uv.y - uvDiff.y;
//
//		if (_uNew < 0)
//		{
//			if (obj->IsWrappedU)
//			{
//				_uNew = 1;
//			}
//			else
//			{
//				_uNew = 0;
//				if (backed)
//				{
//					end = true;
//				}
//				else
//				{
//					backThisTime = true;
//				}
//			}
//		}
//		else if (_uNew > 1)
//		{
//			if (obj->IsWrappedU)
//			{
//				_uNew = 0;
//			}
//			else
//			{
//				_uNew = 1;
//				if (backed)
//				{
//					end = true;
//				}
//				else
//				{
//					backThisTime = true;
//				}
//			}
//		}
//
//		if (_vNew > 1)
//		{
//			if (obj->IsWrappedV)
//			{
//				_vNew = 0;
//			}
//			else
//			{
//				_vNew = 1;
//				if (backed)
//				{
//					end = true;
//				}
//				else
//				{
//					backThisTime = true;
//				}
//			}
//		}
//		else if (_vNew < 0)
//		{
//			if (obj->IsWrappedV)
//			{
//				_vNew = 1;
//			}
//			else
//			{
//				_vNew = 0;
//				if (backed)
//				{
//					end = true;
//				}
//				else
//				{
//					backThisTime = true;
//				}
//			}
//		}
//
//		return  UpdateUVStruct(XMFLOAT2(_uNew, _vNew), end, backThisTime);
//	}
//};
