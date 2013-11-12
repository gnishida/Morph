#pragma once

#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class BFSProp {
public:
	RoadGraph* roads1;
	RoadGraph* roads2;

	BFSTree* tree1;
	BFSTree* tree2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::vector<RoadGraph*> sequence;
	int selected;

public:
	BFSProp(const char* filename1, const char* filename2);
	~BFSProp();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale);
	RoadGraph* interpolate(float t);

	void buildTree();
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2);
	bool findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, RoadVertexDesc& child1, RoadVertexDesc& child2);

	void selectSequence(int selected);
	void clearSequence();
	RoadGraph* copyRoads(RoadGraph* roads, BFSTree* tree, int num);



	void createRoads1();
	void createRoads2();
};

