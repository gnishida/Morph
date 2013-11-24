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
	QMap<RoadVertexDesc, RoadVertexDesc> findMinUnsimilarity(RoadGraph* roads1, RoadGraph* roads2);
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc> map);

};

