#pragma once

#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class BFS {
public:
	RoadGraph* roads1;
	RoadGraph* roads2;

	/*
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree1;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree2;

	RoadVertexDesc root1;
	RoadVertexDesc root2;
	*/

	BFSTree* tree1;
	BFSTree* tree2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::vector<RoadGraph*> sequence;
	int selected;

public:
	BFS(const char* filename1, const char* filename2);
	~BFS();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale);
	RoadGraph* interpolate(float t);

	void buildTree();
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2);
	QMap<RoadVertexDesc, RoadVertexDesc> findPairs(RoadGraph* roads, BFSTree* tree1, RoadVertexDesc parent1, std::vector<RoadVertexDesc> children1, RoadGraph* roads2, BFSTree* tree2, RoadVertexDesc parent2, std::vector<RoadVertexDesc> children2);
	bool findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, std::vector<bool>* paired1, RoadGraph* roads2, RoadVertexDesc parent2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2, std::vector<bool>* paired2, RoadVertexDesc& child1, RoadVertexDesc& child2);

	void selectSequence(int selected);
	void clearSequence();
};

