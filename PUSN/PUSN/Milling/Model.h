#pragma once
#include <d3d11.h>
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

#include "Bezier/BezierSurface.h"
#include "Bezier/IntersectionCurve.h"
#include "../StringHelper.h"

class Model
{
public:
	vector<shared_ptr<BezierSurfaceC0>> model;

	vector<BezierSurfaceC0*> GetSurfaces();

	void LoadElephant(float minZ);

	BezierSurfaceC0* GetTorso();
	BezierSurfaceC0* GetLegFront();
	BezierSurfaceC0* GetLegBack();
	BezierSurfaceC0* GetTail();
	BezierSurfaceC0* GetHead();
	BezierSurfaceC0* GetRightEar();
	BezierSurfaceC0* GetLeftEar();
	BezierSurfaceC0* GetBox();

};

