#pragma once

#include <QVector3D>
#include <vector>

class BBox {
public:
	QVector3D minPt;
	QVector3D maxPt;

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
		minPt.setZ(FLT_MAX);
		maxPt.setX(-FLT_MAX);
		maxPt.setY(-FLT_MAX);
		maxPt.setZ(-FLT_MAX);
	}

	void combineWithBBox(const BBox& other);
	void addPoint(const QVector3D& newPt);

	inline bool overlapsWithBBoxXY(const BBox& other) {
		return  
			( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
			( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) );					
	}

	inline QVector3D midPt() {
		return 0.5 * (minPt + maxPt);
	}

	inline void recalculate(const std::vector<QVector3D>& vertices) {
		minPt.setX(FLT_MAX);
		minPt.setY(FLT_MAX);
		minPt.setZ(FLT_MAX);
		maxPt.setX(-FLT_MAX);
		maxPt.setY(-FLT_MAX);
		maxPt.setZ(-FLT_MAX);

		for (size_t i = 0; i < vertices.size(); ++i) {
			if (vertices[i].x() < minPt.x()) { minPt.setX(vertices[i].x()); }
			if (vertices[i].y() < minPt.y()) { minPt.setY(vertices[i].y()); }
			if (vertices[i].z() < minPt.z()) { minPt.setZ(vertices[i].z()); }

			if (vertices[i].x() > maxPt.x()) { maxPt.setX(vertices[i].x()); }
			if (vertices[i].y() > maxPt.y()) { maxPt.setY(vertices[i].y()); }
			if (vertices[i].z() > maxPt.z()) { maxPt.setZ(vertices[i].z()); }
		}
	}

	bool contains(const QVector3D &pt) const;
};

