#include "RoadEdge.h"

RoadEdge::RoadEdge(unsigned int lanes, unsigned int type, bool oneWay) {
	this->lanes = lanes;
	this->type = type;
	this->oneWay = oneWay;
	this->valid = true;
	this->orig = false;
}

RoadEdge::~RoadEdge() {
}

int RoadEdge::getNumLanes() {
	return lanes;
}

float RoadEdge::getLength() {
	float length = 0.0f;
	for (int i = 0; i < polyLine.size() - 1; i++) {
		length += (polyLine[i + 1] - polyLine[i]).length();
	}

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

float RoadEdge::getWeight() {
	return powf(2, lanes * 2.0f);
}
