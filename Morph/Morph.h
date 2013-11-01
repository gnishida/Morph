#ifndef MORPH_H
#define MORPH_H

#include <QtGui/QMainWindow>
#include "RoadGraph.h"
#include "ui_Morph.h"
#include <qmap.h>
#include <qtimer.h>

class Morph : public QMainWindow {
	Q_OBJECT

private:
	QTimer* timer;
	float t;
	RoadGraph* roadsA;
	RoadGraph* roadsB;
	RoadGraph* interpolated_roads;

	// neighbor
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor1;
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor2;

public:
	Morph(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Morph();

protected:
    void paintEvent(QPaintEvent *);

private:
	Ui::MorphClass ui;

private slots:
	void start();
	void tick();

public:
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2);
	RoadGraph* interpolate(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, float t);
	RoadGraph* buildGraph1();
	RoadGraph* buildGraph2();
	RoadGraph* buildGraph3();
	RoadGraph* buildGraph4();
	RoadGraph* buildGraph5();

	void addNodesOnEdges(RoadGraph* roads, int numNodes);

	// 第１ステップ
	QMap<RoadVertexDesc, RoadVertexDesc> findNearestNeighbors(RoadGraph* roads1, RoadGraph* roads2);

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

	RoadVertexDesc findNearestNeighbor(RoadGraph* roads, QVector2D pt, RoadVertexDesc ignore);

	bool hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2);
	bool hasOriginalEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2);
	RoadEdgeDesc getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt);
	void setupSiblings(RoadGraph* roads);
};


#endif // MORPH_H
