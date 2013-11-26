#pragma once

#include "BFSTree.h"

class BFSForest : public BFSTree {
public:
	QList<RoadVertexDesc> roots;

public:
	BFSForest(RoadGraph* roads, QList<RoadVertexDesc> roots);
	~BFSForest();
	
	QList<RoadVertexDesc> getParent(RoadVertexDesc node);
	QList<RoadVertexDesc> getRoots();
	void buildForest();
};

