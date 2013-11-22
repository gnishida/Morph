#pragma once

#include "AbstractBFS.h"
#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class BFS2 : public AbstractBFS {
public:
	BFSTree* tree1;
	BFSTree* tree2;
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

public:
	BFS2();
	~BFS2();

	RoadGraph* interpolate(float t);

	void init();
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2);
	bool findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2);

	RoadGraph* copyRoads(RoadGraph* roads, BFSTree* tree, int num);

	void findBestRoots(RoadGraph* roads1, RoadGraph* roads2, RoadVertexDesc& root1, RoadVertexDesc& root2);
	int computeUnbalanceness(RoadGraph* roads1, RoadVertexDesc node1, RoadGraph* roads2,  RoadVertexDesc node2);
};

