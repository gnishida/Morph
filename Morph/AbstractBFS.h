#pragma once

#include "RoadGraph.h"
#include "BFSTree.h"
#include <qpainter.h>

class Morph;

class AbstractBFS {
public:
	RoadGraph* roads1;
	RoadGraph* roads2;

	std::vector<RoadGraph*> sequence;
	int selected;

public:
	AbstractBFS(const char* filename1, const char* filename2);
	~AbstractBFS();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale, bool label = false);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale);
	
	void selectSequence(int selected);
	void clearSequence();

	void createRoads1();
	void createRoads2();
	void createRoads3();
	void createRoads4();
};

