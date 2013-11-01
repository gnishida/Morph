#include "RoadEdge.h"

RoadEdge::RoadEdge(int lanes, int type) {
	this->lanes = lanes;
	this->type = type;
	this->length = 0.0f;
	this->valid = true;
}

RoadEdge::~RoadEdge() {
}

int RoadEdge::getNumLanes() {
	return lanes;
}

float RoadEdge::getLength() {
	return length;
}

int RoadEdge::getType() {
	return type;
}

std::vector<QVector2D> RoadEdge::getPolyLine() {
	return polyLine;
}

/**
 * Add a point to the polyline of the road segment.
 *
 * @param pt		new point to be added
 */
void RoadEdge::addPoint(const QVector2D &pt) {
	polyLine.push_back(pt);
}

float RoadEdge::getWidth() {
	return lanes * 2 * 3.5f;
}

