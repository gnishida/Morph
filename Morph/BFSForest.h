#pragma once

#include "RoadGraph.h"
#include <vector>

class BFSForest {
public:
	RoadGraph* roads;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > children;

	QList<RoadVertexDesc> roots;

public:
	BFSForest(RoadGraph* roads, QList<RoadVertexDesc> roots);
	~BFSForest();
	
	std::vector<RoadVertexDesc>& getChildren(RoadVertexDesc node);
	void addChild(RoadVertexDesc parent, RoadVertexDesc child);

	QList<RoadVertexDesc> getParent(RoadVertexDesc node);
	QList<RoadVertexDesc> getRoots();
	void buildForest();
};

