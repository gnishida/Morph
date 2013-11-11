#pragma once

#include "RoadGraph.h"

class BFSTree {
private:
	RoadGraph* roads;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > children;
	RoadVertexDesc root;

public:
	BFSTree(RoadGraph* roads, RoadVertexDesc root);
	~BFSTree();

	std::vector<RoadVertexDesc> getChildren(RoadVertexDesc node);
	RoadVertexDesc getRoot();
	int getHeight(RoadVertexDesc node);
	RoadVertexDesc copySubTree(RoadVertexDesc node1, RoadVertexDesc node2);

private:
	void buildTree();
};

