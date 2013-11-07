#pragma once

#include "RoadGraph.h"
#include "BBox.h"
#include <qmap.h>

class Morphing2 {
public:
	RoadGraph* roadsA;
	RoadGraph* roadsB;

	// neighbor
	QMap<RoadVertexDesc, QSet<RoadVertexDesc>* > correspond1;
	QMap<RoadVertexDesc, QSet<RoadVertexDesc>* > correspond2;

public:
	Morphing2();
	~Morphing2();

	void initRoads(const char* filename1, const char* filename2);
	RoadGraph* interpolate(float t);

	void initCorrespondence(RoadGraph* roads, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspondence);
	void findBestPairs(RoadGraph* roads1, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* neighbor2);
	void propagatePairs(RoadGraph* roads1, RoadVertexDesc v1_desc, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond1, RoadGraph* roads2, RoadVertexDesc v2_desc, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond2);
};

