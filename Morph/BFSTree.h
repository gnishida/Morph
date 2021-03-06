#pragma once

#include "RoadGraph.h"

class BFSTree {
public:
	RoadGraph* roads;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > children;
	RoadVertexDesc root;

public:
	BFSTree();
	BFSTree(RoadGraph* roads, RoadVertexDesc root);
	~BFSTree();

	std::vector<RoadVertexDesc>& getChildren(RoadVertexDesc node);
	void addChild(RoadVertexDesc parent, RoadVertexDesc child);
	RoadVertexDesc getParent(RoadVertexDesc node);
	RoadVertexDesc getRoot();
	int getHeight(RoadVertexDesc node);
	RoadVertexDesc copySubTree(RoadVertexDesc node1_parent, RoadVertexDesc node1, RoadVertexDesc node2);
	void removeSubTree(RoadVertexDesc node1);
	int getTreeSize(RoadVertexDesc node);

private:
	void buildTree();
};

