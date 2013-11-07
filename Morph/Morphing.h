#pragma once

#include "RoadGraph.h"
#include "BBox.h"
#include <qmap.h>
#include <qpainter.h>

class Morph;

class Morphing {
public:
	Morph* morph;
	RoadGraph* roadsA;
	RoadGraph* roadsB;

	// neighbor
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor1;
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor2;

public:
	Morphing(Morph* morph);
	~Morphing();

	void draw(QPainter* painter, float t, int offset, float scale);
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);

	void initRoads(const char* filename1, const char* filename2);
	RoadGraph* interpolate(float t);

	void addNodesOnEdges(RoadGraph* roads, int numNodes);

	// 第１ステップ
	QMap<RoadVertexDesc, RoadVertexDesc> findNearestNeighbors(RoadGraph* roads1, RoadGraph* roads2, int width, int height, int cellLength);
	void findBestPairs(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, float threshold);
	void propagatePairs(RoadGraph* roads1, RoadVertexDesc v1_desc, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, RoadVertexDesc v2_desc, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);

	// 第２ステップ
	void checkExclusivePair(RoadGraph* roads, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);

	// 第３ステップ
	void changeAloneToPair(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);

	// 第４ステップ
	void augmentGraph(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);

	// 第５ステップ
	void updateEdges(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);
	
	// 第６ステップ
	void updateEdges2(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2);

	bool updateEdgeWithSibling1(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt);
	bool updateEdgeWithSibling2(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt);
	bool updateEdgeWithSibling3(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt);
	bool updateEdgeWithSibling4(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt);
	void updateEdgeWithSplit(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt);

	RoadVertexDesc findNearestNeighbor(RoadGraph* roads, const QVector2D &pt, const BBox &area);
	RoadVertexDesc findBestConnectedNeighbor(RoadGraph* roads1, RoadVertexDesc v1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, float threshold);

	bool hasOriginalEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2);
	RoadEdgeDesc getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt);
	void setupSiblings(RoadGraph* roads);
	
	RoadGraph* buildGraph1();
	RoadGraph* buildGraph2();
	RoadGraph* buildGraph3();
	RoadGraph* buildGraph4();
	RoadGraph* buildGraph5();
};

