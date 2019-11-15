#include "IntersectionCurve.h"

int   IntersectionCurve::_count = 0;

//float IntersectionCurve::_gradientEps = 0.0001f;
//float IntersectionCurve::_startGradientAlpha = 0.01f;
//
//float IntersectionCurve::_newtonStartAlpha = 0.002f;
//float IntersectionCurve::_finalEpsilon = 0.01f;
//float IntersectionCurve::_alphaEpsilon = 0.001f;

float IntersectionCurve::_gradientEps = 0.0001f;
float IntersectionCurve::_startGradientAlpha = 0.01f;

float IntersectionCurve::_newtonStartAlpha = 0.2f;
float IntersectionCurve::_finalEpsilon = 0.01f;
float IntersectionCurve::_alphaEpsilon = 0.001f;