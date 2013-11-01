#pragma once

#include <qvector2d.h>

class RoadVertex {
public:
	QVector2D pt;
	int ref;

public:
	RoadVertex();
	RoadVertex(const QVector2D &pt);

	const QVector2D& getPt() const;
};

