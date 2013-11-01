#pragma once

#include <qvector2d.h>
#include <vector>

class RoadVertex;

class RoadEdge {
public:
	unsigned int lanes;
	float length;
	unsigned int type;
	std::vector<QVector2D> polyLine;



public:
	RoadEdge(int numLanes, int typeRoad);
	~RoadEdge();
	
	int getNumLanes();
	float getLength();
	int getType();
	std::vector<QVector2D> getPolyLine();

	void addPoint(const QVector2D &pt);
	float getWidth();

};

