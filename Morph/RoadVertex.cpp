#include "RoadVertex.h"

RoadVertex::RoadVertex() {
	this->pt = QVector2D(0.0f, 0.0f);
	this->orig = true;
	this->virt = false;
}

RoadVertex::RoadVertex(const QVector2D &pt) {
	this->pt = pt;
	this->orig = true;
	this->virt = false;
}

const QVector2D& RoadVertex::getPt() const {
	return pt;
}