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
	AbstractBFS();
	~AbstractBFS();
	
	void setRoad1(const char* filename);
	void setRoad2(const char* filename);

	virtual void init() = 0;

	void draw(QPainter* painter);
	void drawGraph(QPainter *painter, RoadGraph *roads, int size, bool label = false);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2);
	
	void selectSequence(int selected);
	void clearSequence();
	RoadGraph* getSelectedRoads();

	void createRoads1();
	void createRoads2();
	void createRoads3();
	void createRoads4();
	
	void createRoads5();
	void createRoads6();
};

