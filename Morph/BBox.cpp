#include "BBox.h"
#include <limits>

BBox::BBox() {
	minPt.setX(std::numeric_limits<float>::max());
	minPt.setY(std::numeric_limits<float>::max());
	maxPt.setX(-std::numeric_limits<float>::max());
	maxPt.setY(-std::numeric_limits<float>::max());
}

BBox::~BBox() {
}

/**
 * update the bounding box by combining aother bounding box.
 *
 * @param other aother bounding box
 */
void BBox::combineWithBBox(const BBox& other) {	
	minPt.setX(qMin(minPt.x(), other.minPt.x()));
	minPt.setY(qMin(minPt.y(), other.minPt.y()));

	maxPt.setX(qMax(maxPt.x(), other.maxPt.x()));
	maxPt.setY(qMax(maxPt.y(), other.maxPt.y()));
}

/**
 * update the bounding box by adding a new point.
 *
 * @param newPt new point
 */
void BBox::addPoint(const QVector2D& newPt) {
	minPt.setX(qMin(minPt.x(), newPt.x()));
	minPt.setY(qMin(minPt.y(), newPt.y()));

	maxPt.setX(qMax(maxPt.x(), newPt.x()));
	maxPt.setY(qMax(maxPt.y(), newPt.y()));
}

bool BBox::contains(const QVector2D &pt) const {
	if (pt.x() < minPt.x() || pt.y() < minPt.y()) return false;
	if (pt.x() > maxPt.x() || pt.y() > maxPt.y()) return false;

	return true;
}
