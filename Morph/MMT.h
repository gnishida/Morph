#pragma once

#include "RoadGraph.h"
#include <qpainter.h>

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
	RoadGraph* roads1;
	RoadGraph* roads2;

	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree1;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

public:
	MMT(Morph* morph, const char* filename1, const char* filename2);

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void buildTree();

	void buildTree2();
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > bfs(RoadGraph* roads, RoadVertexDesc root);
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, RoadVertexDesc root1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, RoadGraph* roads2, RoadVertexDesc root2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2);
	bool findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, std::vector<RoadVertexDesc>* children1, std::vector<bool>* paired1, RoadGraph* roads2, RoadVertexDesc parent2, std::vector<RoadVertexDesc>* children2, std::vector<bool>* paired2, RoadVertexDesc& child1, RoadVertexDesc& child2);

	void collapse(RoadGraph* roads);
	void expand(RoadGraph* roads);
};

