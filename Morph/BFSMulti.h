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
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, BFSForest* forest1, RoadGraph* roads2, BFSForest* forest2);
	bool findBestPairByDirection(float theta, RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, bool> paired2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2);
};

