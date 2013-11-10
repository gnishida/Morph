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
	RoadGraph* roads1;
	RoadGraph* roads2;

	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree1;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree2;

	RoadVertexDesc root1;
	RoadVertexDesc root2;

	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::vector<RoadGraph*> sequence1;
	std::vector<RoadGraph*> sequence2;

	int selected;

public:
	MTT(Morph* morph, const char* filename1, const char* filename2);

	void draw(QPainter* painter, float t, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);

	void buildTree();

	void collapse(RoadGraph* roads, std::vector<RoadGraph*>* sequence);
	void expand(RoadGraph* roads);

	void findBoundaryVertices(RoadGraph* roads, RoadVertexDesc &v1_desc, RoadVertexDesc &v2_desc);
	void createVertexMatrix(RoadGraph* roads, cv::Mat& vmat);

	void selectSequence(int selected);
};

