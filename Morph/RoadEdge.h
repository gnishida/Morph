#pragma once

#include <qvector2d.h>
#include <vector>

class RoadVertex;

class RoadEdge {
public:
	unsigned int lanes;
	//float length;
	unsigned int type;
	bool oneWay;
	std::vector<QVector2D> polyLine;
	float weight;
	bool valid;
	bool orig;

public:
	RoadEdge(unsigned int lanes, unsigned int type, bool oneWay);
	~RoadEdge();
	
	int getNumLanes();
	float getLength();
	int getType();
	std::vector<QVector2D> getPolyLine();

	void addPoint(const QVector2D &pt);
	float getWidth();
};

