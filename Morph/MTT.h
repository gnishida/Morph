#pragma once

#include "RoadGraph.h"
#include "PCA.h"
#include <qpainter.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class Morph;

class MTT {
public:
	RoadGraph* roads;

	std::vector<RoadGraph*> sequence;

	int selected;

public:
	MTT(const char* filename);
	~MTT();

	void draw(QPainter* painter, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);

	void buildTree(float w1, float w2, float w3);

	void collapse(RoadGraph* roads, float w1, float w2, float w3, std::vector<RoadGraph*>* sequence);
	float getMetric(RoadGraph* roads, RoadVertexDesc v1_desc, RoadVertexDesc v2_desc, float w1, float w2, float w3, PCA& pca);

	void expand(RoadGraph* roads);

	void findBoundaryVertices(RoadGraph* roads, RoadVertexDesc &v1_desc, RoadVertexDesc &v2_desc);
	void doPCA(RoadGraph* roads, PCA& pca);

	void selectSequence(int selected);
	void clearSequence();
};

