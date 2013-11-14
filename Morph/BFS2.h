#pragma once

#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class BFS2 {
public:
	RoadGraph* roads1;
	RoadGraph* roads2;

	BFSTree* tree1;
	BFSTree* tree2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::vector<RoadGraph*> sequence;
	int selected;

public:
	BFS2(const char* filename1, const char* filename2);
	~BFS2();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale);
	RoadGraph* interpolate(float t);

	void buildTree();
	QMap<RoadVertexDesc, RoadVertexDesc> findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2);
	bool findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2);

	void selectSequence(int selected);
	void clearSequence();
	RoadGraph* copyRoads(RoadGraph* roads, BFSTree* tree, int num);

	void findBestRoots(RoadGraph* roads1, RoadGraph* roads2, RoadVertexDesc& root1, RoadVertexDesc& root2);
	int computeUnbalanceness(RoadGraph* roads1, RoadVertexDesc node1, RoadGraph* roads2,  RoadVertexDesc node2);

	void createRoads1();
	void createRoads2();
};

