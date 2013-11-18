#pragma once

#include <qvector2d.h>

#ifndef M_PI
#define M_PI	3.14159265
#endif

class Util {
public:
	static const float MTC_FLOAT_TOL;

public:
	Util();
	~Util();

	static bool segmentSegmentIntersectXY(const QVector2D& a, const QVector2D& b, const QVector2D& c, const QVector2D& d, float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint);
	static float pointSegmentDistanceXY(const QVector2D& a, const QVector2D& b, const QVector2D& c, QVector2D& closestPtInAB);
};

