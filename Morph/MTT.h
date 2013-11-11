#pragma once

#include "RoadGraph.h"
#include <qpainter.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

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

class MTT {
public:
	Morph* morph;
	RoadGraph* roads;

	//QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree1;
	//QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree2;

	//RoadVertexDesc root1;
	//RoadVertexDesc root2;

	//QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::vector<RoadGraph*> sequence;

	int selected;

public:
	MTT(const char* filename);
	~MTT();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);

	void buildTree();

	void collapse(RoadGraph* roads, std::vector<RoadGraph*>* sequence);
	void expand(RoadGraph* roads);

	void findBoundaryVertices(RoadGraph* roads, RoadVertexDesc &v1_desc, RoadVertexDesc &v2_desc);
	void createVertexMatrix(RoadGraph* roads, cv::Mat& vmat);

	void selectSequence(int selected);
};

