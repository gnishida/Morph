#pragma once

#include <qvector2d.h>

class Util {
public:
	static const float MTC_FLOAT_TOL;

public:
	Util();
	~Util();

	static bool segmentSegmentIntersectXY(const QVector2D& a, const QVector2D& b, const QVector2D& c, const QVector2D& d, float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint);
};

