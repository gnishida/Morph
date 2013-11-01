#pragma once

#include <vector>
#include <qvector2d.h>

class RoadVertex {
public:
	QVector2D pt;
	int ref;
	bool orig;
	bool virt;
	bool pair;
	bool finalized;

public:
	RoadVertex();
	RoadVertex(const QVector2D &pt);

	const QVector2D& getPt() const;
};

