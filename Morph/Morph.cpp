#include "Morph.h"
#include "BBox.h"
#include "MMT.h"
#include <qset.h>
#include <qpainter.h>
#include <qcolor.h>
#include <limits>
#include <time.h>
#include <qdebug.h>
#include <boost/graph/graph_utility.hpp>

Morph::Morph(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	timer = new QTimer(this);

	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	width = height = 2000;
	//width = height = 10000;
	//cellLength = 1000;

	morphing = NULL;
	morphing2 = NULL;
	roads = NULL;
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	/*
	drawGraph(&painter, roads1, QColor(0, 0, 255));
	drawGraph(&painter, roads2, QColor(255, 0, 0));
	drawRelation(&painter, roads1, neighbor1, roads2, neighbor2);
	*/

	//drawGraph(&painter, interpolated_roads, QColor(0, 0, 255), width / 2 + 150, 800.0f / width);
	drawGraph(&painter, roads, QColor(0, 0, 255), width / 2 + 150, 800.0f / width);
}

void Morph::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
	if (roads == NULL) return;

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(QPen(col, 1, Qt::SolidLine, Qt::RoundCap));
	painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* edge = roads->graph[*ei];
		if (!edge->valid) continue;

		for (int i = 0; i < edge->getPolyLine().size() - 1; i++) {
			int x1 = (edge->getPolyLine()[i].x() + offset) * scale;
			int y1 = (-edge->getPolyLine()[i].y() + offset) * scale;
			int x2 = (edge->getPolyLine()[i+1].x() + offset) * scale;
			int y2 = (-edge->getPolyLine()[i+1].y() + offset) * scale;
			painter->drawLine(x1, y1, x2, y2);
		}
	}

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		RoadVertex* v = roads->graph[*vi];

		int x = (v->getPt().x() + offset) * scale;
		int y = (-v->getPt().y() + offset) * scale;
		painter->fillRect(x - 1, y - 1, 3, 3, col);
	}
}

void Morph::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2) {
	if (roads1 == NULL || roads2 == NULL) return;

	painter->setPen(QPen(Qt::black, 2, Qt::DotLine, Qt::RoundCap));

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		RoadVertex* v2 = roads2->graph[neighbor1[*vi]];

		painter->drawLine(v1->getPt().x(), v1->getPt().y(), v2->getPt().x(), v2->getPt().y());
	}
}

void Morph::start() {
	if (morphing2 == NULL) {
		//initRoads("london_10000.gsm", "paris_10000.gsm");	
		morphing2 = new Morphing2();
		morphing2->initRoads("roads1.gsm", "roads2.gsm");
	}

	t = 1.0f;

	timer->start(100);

	/*
	MMT mmt(this, "roads1.gsm");
	mmt.buildTree();
	*/
}

void Morph::tick() {
	if (roads != NULL) {
		roads->clear();
		delete roads;
	}
	roads = morphing2->interpolate(t);

	update();

	t -= 0.02f;
	if (t < 0.0f) {
		t = 0.0f;
		timer->stop();
	}
}

