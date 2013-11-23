#pragma once

#include "AbstractBFS.h"
#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class BFSProp : public AbstractBFS {
public:
	BFSTree* tree1;
	BFSTree* tree2;
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

public:
	BFSProp();
	~BFSProp();

	RoadGraph* interpolate(float t);

	void init();
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, QList<RoadEdgeDesc>& chosen1, RoadGraph* roads2, QList<RoadEdgeDesc>& chosen2);
	bool findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, RoadVertexDesc& child1, RoadVertexDesc& child2);

};

