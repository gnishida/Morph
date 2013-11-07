#pragma once

#include "RoadGraph.h"
#include <qpainter.h>

class Morph;

class BFS {
private:
	Morph* morph;
	RoadGraph* roads1;
	RoadGraph* roads2;

	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree1;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree2;

	RoadVertexDesc root1;
	RoadVertexDesc root2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

public:
	BFS(Morph* morph, const char* filename1, const char* filename2);

	void draw(QPainter* painter, float t, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	RoadGraph* interpolate(float t);

	void buildTree();
	void bfs(RoadGraph* roads, RoadVertexDesc root, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree);
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, RoadVertexDesc root1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, RoadGraph* roads2, RoadVertexDesc root2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2);
	bool findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, std::vector<bool>* paired1, RoadGraph* roads2, RoadVertexDesc parent2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2, std::vector<bool>* paired2, RoadVertexDesc& child1, RoadVertexDesc& child2);

	void collapse(RoadGraph* roads);
	void expand(RoadGraph* roads);
};

