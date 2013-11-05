#pragma once

#include "RoadGraph.h"

class Morph;

class VertexPriority {
public:
	RoadVertexDesc desc;
	float priority;

public:
	VertexPriority();
	VertexPriority(RoadVertexDesc desc, float priority);
};

struct dataComparison {
	bool operator () (VertexPriority left, VertexPriority right) {
		return left.priority > right.priority; 
	}
};

class MMT {
private:
	Morph* morph;
	RoadGraph* roads;

public:
	MMT(Morph* morph, const char* filename);

	void buildTree();
	void collapse(RoadGraph* roads);
};

