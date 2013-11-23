#pragma once

#include <QVector2D>
#include <vector>

class BBox {
public:
	QVector2D minPt;
	QVector2D maxPt;

public:
	BBox();
	~BBox();

	inline BBox(const BBox &ref) {	
		minPt = ref.minPt;
		maxPt = ref.maxPt;
	}

	inline BBox& operator=(const BBox &ref) {
		minPt = ref.minPt;
		maxPt = ref.maxPt;				
		return (*this);
	}

	inline void reset() {
		minPt.setX(FLT_MAX);
		minPt.setY(FLT_MAX);
		maxPt.setX(-FLT_MAX);
		maxPt.setY(-FLT_MAX);
	}

	void combineWithBBox(const BBox& other);
	void addPoint(const QVector2D& newPt);

	inline bool overlapsWithBBoxXY(const BBox& other) {
		return  
			( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
			( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) );					
	}

	inline QVector2D midPt() {
		return 0.5 * (minPt + maxPt);
	}

	inline float dx() {
		return maxPt.x() - minPt.x();
	}

	inline float dy() {
		return maxPt.y() - minPt.y();
	}

	inline void recalculate(const std::vector<QVector2D>& vertices) {
		minPt.setX(FLT_MAX);
		minPt.setY(FLT_MAX);
		maxPt.setX(-FLT_MAX);
		maxPt.setY(-FLT_MAX);

		for (size_t i = 0; i < vertices.size(); ++i) {
			if (vertices[i].x() < minPt.x()) { minPt.setX(vertices[i].x()); }
			if (vertices[i].y() < minPt.y()) { minPt.setY(vertices[i].y()); }

			if (vertices[i].x() > maxPt.x()) { maxPt.setX(vertices[i].x()); }
			if (vertices[i].y() > maxPt.y()) { maxPt.setY(vertices[i].y()); }
		}
	}

	bool contains(const QVector2D &pt) const;
};

