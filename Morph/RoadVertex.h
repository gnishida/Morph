#pragma once

#include <qvector2d.h>

class RoadVertex {
public:
	QVector2D pt;
	int ref;
	bool orig;
	bool virt;

public:
	RoadVertex();
	RoadVertex(const QVector2D &pt);

	const QVector2D& getPt() const;
};

