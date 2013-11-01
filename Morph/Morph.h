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
	RoadGraph* roads1;
	RoadGraph* roads2;
	RoadGraph* roads;
	RoadGraph* new_roads1;
	RoadGraph* new_roads2;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > corr1;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > corr2;

	// neighbor
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor1;
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor2;

	// siblings
	//QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > sibling1;
	//QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > sibling2;

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
	void drawGraph(QPainter *painter, RoadGraph *roads);
	RoadGraph* interpolate(RoadGraph* roads1, RoadGraph* roads2, float t);
	RoadGraph* buildGraph1();
	RoadGraph* buildGraph2();
	void buildCorrespondence(RoadGraph* roads1, RoadGraph* roads2);
	void findNearestNeighbors(RoadGraph* roads1, RoadGraph* roads2);
	void augmentGraph();
	void findExclusiveNearestNeighbor(RoadGraph* roads1, RoadGraph* roads2);
	RoadVertexDesc findNearestNeighbor(RoadGraph* roads, QVector2D pt, RoadVertexDesc ignore);
	void buildEdges(RoadGraph* roads1, RoadGraph* roads2);
	RoadGraph* copyGraphVertices(RoadGraph* roads);

	bool isSibling(QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > sibling, RoadVertexDesc desc1, RoadVertexDesc desc2);
	bool hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2);
	bool hasExclusiveEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2);
};


#endif // MORPH_H
