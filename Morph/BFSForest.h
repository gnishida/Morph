#pragma once

#include "BFSTree.h"

class BFSForest : public BFSTree {
public:
	std::vector<RoadVertexDesc> roots;

public:
	BFSForest(RoadGraph* roads, std::vector<RoadVertexDesc> roots);
	~BFSForest();
	
	std::vector<RoadVertexDesc> getRoots();
	void buildForest();
};

