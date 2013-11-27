#pragma once

#include "AbstractBFS.h"
#include "RoadGraph.h"
#include "BFSForest.h"
#include <qpainter.h>

class Morph;

class BFSMulti : public AbstractBFS {
public:
	BFSForest* forest1;
	BFSForest* forest2;
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

public:
	BFSMulti();
	~BFSMulti();

	RoadGraph* interpolate(float t);

	void init();
	void findCorrespondence(RoadGraph* roads1, BFSForest* forest1, RoadGraph* roads2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2);
	bool findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2);

	float computeDissimilarity(RoadGraph* roads1, QList<RoadVertexDesc> seeds1, RoadGraph* roads2, QList<RoadVertexDesc> seeds2, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2);
};

