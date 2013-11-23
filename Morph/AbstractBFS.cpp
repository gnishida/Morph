#include "AbstractBFS.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

AbstractBFS::AbstractBFS() {
	roads1 = NULL;
	roads2 = NULL;
	selected = 0;
}

AbstractBFS::~AbstractBFS() {
	delete roads1;
	delete roads2;
	clearSequence();
}

void AbstractBFS::setRoad1(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	roads1 = new RoadGraph();
	roads1->load(fp, 2);
	//GraphUtil::planarify(roads1);
	GraphUtil::singlify(roads1);
	GraphUtil::simplify(roads1, 100);
	GraphUtil::reduce(roads1);
	GraphUtil::removeDeadEnd(roads1);
	fclose(fp);

	// 道路のヒストグラム情報を出力
	GraphUtil::printStatistics(roads1);

	if (roads2 != NULL) {
		init();
	} else {
		// バネの原理で、normalizeする
		BBox area;
		area.addPoint(QVector2D(-5000, -5000));
		area.addPoint(QVector2D(5000, 5000));

		clearSequence();
		for (int i = 0; i < 100; i++) {
			GraphUtil::normalizeBySpring(roads1, area);
			GraphUtil::getBoudingBox(roads1, -M_PI, M_PI);
			GraphUtil::scaleToBBox(roads1, area);
			sequence.push_back(GraphUtil::copyRoads(roads1));
		}
	}
}

void AbstractBFS::setRoad2(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	roads2 = new RoadGraph();
	roads2->load(fp, 2);
	//GraphUtil::planarify(roads1);
	GraphUtil::singlify(roads2);
	GraphUtil::simplify(roads2, 100);
	GraphUtil::reduce(roads2);
	GraphUtil::removeDeadEnd(roads2);
	fclose(fp);

	// 道路のヒストグラム情報を出力
	//GraphUtil::printStatistics(roads2);

	if (roads1 != NULL) {
		init();
	}
}

void AbstractBFS::draw(QPainter* painter) {
	if (selected < 0 || selected >= sequence.size()) return;

	//drawGraph(painter, roads1, 50, true);
	//drawGraph(painter, roads2, 50, true);
	//drawRelation(painter, roads1, &correspondence, roads2);

	drawGraph(painter, sequence[selected], 50, true);
}

void AbstractBFS::drawGraph(QPainter *painter, RoadGraph *roads, int size, bool label) {
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* edge = roads->graph[*ei];
		if (!edge->valid) continue;

		if (edge->lanes >= 4) {
			painter->setPen(QPen(QColor(0, 0, 192), 30, Qt::SolidLine, Qt::RoundCap));
		} else {
			painter->setPen(QPen(QColor(192, 192, 255), 8, Qt::SolidLine, Qt::RoundCap));
		}

		for (int i = 0; i < edge->getPolyLine().size() - 1; i++) {
			int x1 = edge->getPolyLine()[i].x();
			int y1 = -edge->getPolyLine()[i].y();
			int x2 = edge->getPolyLine()[i+1].x();
			int y2 = -edge->getPolyLine()[i+1].y();
			painter->drawLine(x1, y1, x2, y2);
		}
	}


	QFont font = painter->font();
	font.setPixelSize(20);
	//font.setPointSize(font.getPointSize() * 10);
	painter->setFont(font);


	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		RoadVertex* v = roads->graph[*vi];
		if (!v->valid) continue;

		int x = v->getPt().x();
		int y = -v->getPt().y();
		painter->fillRect(x - size/2, y - size/2, size, size, QColor(128, 128, 255));

		if (label) {
			// 頂点番号をラベルとして表示する
			QString str;
			str.setNum(*vi);
			painter->drawText(x+2, y+13, str);
		}
	}
}

void AbstractBFS::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2) {
	if (roads1 == NULL || roads2 == NULL) return;

	painter->setPen(QPen(Qt::black, 1, Qt::DotLine, Qt::RoundCap));

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		if (!v1->valid) continue;

		//if (rand() * 10 < 8) continue;

		RoadVertexDesc v2_desc = correspondence->value(*vi);

		RoadVertex* v2 = roads2->graph[v2_desc];
		if (!v2->valid) continue;

		int x1 = v1->getPt().x();
		int y1 = -v1->getPt().y();
		int x2 = v2->getPt().x();
		int y2 = -v2->getPt().y();
		painter->drawLine(x1, y1, x2, y2);
	}
}

void AbstractBFS::selectSequence(int selected) {
	this->selected = selected;
}

void AbstractBFS::clearSequence() {
	for (int i = 0; i < sequence.size(); i++) {
		sequence[i]->clear();
		delete sequence[i];
	}
	sequence.clear();
}

void AbstractBFS::createRoads1() {
	roads1 = new RoadGraph();
	RoadVertex* v1 = new RoadVertex(QVector2D(-900, 900));
	RoadVertexDesc v1_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-800, 700));
	RoadVertexDesc v2_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-100, 400));
	RoadVertexDesc v3_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(250, 650));
	RoadVertexDesc v4_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(400, -200));
	RoadVertexDesc v5_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(900, 200));
	RoadVertexDesc v6_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(600, -400));
	RoadVertexDesc v7_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(200, -900));
	RoadVertexDesc v8_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v8_desc] = v8;

	GraphUtil::addEdge(roads1, v1_desc, v2_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v2_desc, v3_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v1_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v3_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v3_desc, v5_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v5_desc, v6_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v5_desc, v7_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v7_desc, v8_desc, 1, 1, false);
}

void AbstractBFS::createRoads2() {
	roads2 = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(-700, 900));
	RoadVertexDesc v1_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-600, 600));
	RoadVertexDesc v2_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-900, 200));
	RoadVertexDesc v3_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-700, 100));
	RoadVertexDesc v4_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(-800, -200));
	RoadVertexDesc v5_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(-600, -200));
	RoadVertexDesc v6_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(-800, -700));
	RoadVertexDesc v7_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(-500, -800));
	RoadVertexDesc v8_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(-50, 350));
	RoadVertexDesc v9_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(100, 600));
	RoadVertexDesc v10_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(600, 0));
	RoadVertexDesc v11_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(900, 300));
	RoadVertexDesc v12_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(800, -100));
	RoadVertexDesc v13_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(400, -600));
	RoadVertexDesc v14_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(500, -900));
	RoadVertexDesc v15_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(800, -400));
	RoadVertexDesc v16_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v16_desc] = v16;

	GraphUtil::addEdge(roads2, v1_desc, v2_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v2_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v3_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v4_desc, v5_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v4_desc, v6_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v5_desc, v7_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v6_desc, v7_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v6_desc, v8_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v2_desc, v9_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v9_desc, v10_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v9_desc, v11_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v12_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v13_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v14_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v14_desc, v15_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v13_desc, v16_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v15_desc, v16_desc, 1, 1, false);
}

void AbstractBFS::createRoads3() {
	roads1 = new RoadGraph();

	RoadVertex* v0 = new RoadVertex(QVector2D(-750, 750));
	RoadVertexDesc v0_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v0_desc] = v0;

	RoadVertex* v1 = new RoadVertex(QVector2D(-800, 550));
	RoadVertexDesc v1_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-550, 950));
	RoadVertexDesc v2_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-500, 750));
	RoadVertexDesc v3_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-500, 550));
	RoadVertexDesc v4_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(-300, 800));
	RoadVertexDesc v5_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(-200, 700));
	RoadVertexDesc v6_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(-500, 150));
	RoadVertexDesc v7_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(-950, 300));
	RoadVertexDesc v8_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(-950, -500));
	RoadVertexDesc v9_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(-750, -570));
	RoadVertexDesc v10_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(-700, -750));
	RoadVertexDesc v11_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(-400, -700));
	RoadVertexDesc v12_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(-250, -750));
	RoadVertexDesc v13_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(-200, -50));
	RoadVertexDesc v14_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(-50, 0));
	RoadVertexDesc v15_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(500, -500));
	RoadVertexDesc v16_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v16_desc] = v16;

	RoadVertex* v17 = new RoadVertex(QVector2D(450, -750));
	RoadVertexDesc v17_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v17_desc] = v17;

	RoadVertex* v18 = new RoadVertex(QVector2D(750, -750));
	RoadVertexDesc v18_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v18_desc] = v18;

	RoadVertex* v19 = new RoadVertex(QVector2D(550, -300));
	RoadVertexDesc v19_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v19_desc] = v19;

	RoadVertex* v20 = new RoadVertex(QVector2D(700, -200));
	RoadVertexDesc v20_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v20_desc] = v20;

	RoadVertex* v21 = new RoadVertex(QVector2D(950, -350));
	RoadVertexDesc v21_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v21_desc] = v21;

	RoadVertex* v22 = new RoadVertex(QVector2D(600, -150));
	RoadVertexDesc v22_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v22_desc] = v22;

	RoadVertex* v23 = new RoadVertex(QVector2D(500, -50));
	RoadVertexDesc v23_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v23_desc] = v23;

	RoadVertex* v24 = new RoadVertex(QVector2D(550, 450));
	RoadVertexDesc v24_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v24_desc] = v24;

	RoadVertex* v25 = new RoadVertex(QVector2D(900, 250));
	RoadVertexDesc v25_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v25_desc] = v25;

	RoadVertex* v26 = new RoadVertex(QVector2D(250, 600));
	RoadVertexDesc v26_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v26_desc] = v26;

	RoadVertex* v27 = new RoadVertex(QVector2D(150, 650));
	RoadVertexDesc v27_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v27_desc] = v27;

	RoadVertex* v28 = new RoadVertex(QVector2D(250, 950));
	RoadVertexDesc v28_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v28_desc] = v28;

	RoadVertex* v29 = new RoadVertex(QVector2D(400, 950));
	RoadVertexDesc v29_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v29_desc] = v29;

	GraphUtil::addEdge(roads1, v0_desc, v1_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v0_desc, v3_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v1_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v2_desc, v3_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v3_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v3_desc, v5_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v3_desc, v6_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v4_desc, v6_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v4_desc, v7_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v6_desc, v27_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v7_desc, v8_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v7_desc, v14_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v8_desc, v10_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v9_desc, v10_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v10_desc, v11_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v10_desc, v12_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v12_desc, v13_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v12_desc, v14_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v13_desc, v15_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v14_desc, v15_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v14_desc, v27_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v15_desc, v16_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v15_desc, v22_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v15_desc, v26_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v16_desc, v17_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v16_desc, v18_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v16_desc, v19_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v19_desc, v20_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v19_desc, v22_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v20_desc, v21_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v20_desc, v22_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v22_desc, v23_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v22_desc, v24_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v24_desc, v25_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v24_desc, v26_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v26_desc, v27_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v26_desc, v29_desc, 1, 1, false);
	GraphUtil::addEdge(roads1, v27_desc, v28_desc, 1, 1, false);
}


void AbstractBFS::createRoads4() {
	roads2 = new RoadGraph();

	RoadVertex* v0 = new RoadVertex(QVector2D(-750, 550));
	RoadVertexDesc v0_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v0_desc] = v0;

	RoadVertex* v1 = new RoadVertex(QVector2D(-800, 350));
	RoadVertexDesc v1_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-550, 380));
	RoadVertexDesc v2_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-400, 50));
	RoadVertexDesc v3_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-900, -250));
	RoadVertexDesc v4_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(0, 0));
	RoadVertexDesc v5_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(0, -400));
	RoadVertexDesc v6_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(-550, -450));
	RoadVertexDesc v7_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(-750, -300));
	RoadVertexDesc v8_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(-800, -800));
	RoadVertexDesc v9_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(0, -700));
	RoadVertexDesc v10_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(700, -400));
	RoadVertexDesc v11_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(750, -700));
	RoadVertexDesc v12_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(860, -350));
	RoadVertexDesc v13_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(300, -400));
	RoadVertexDesc v14_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(500, -100));
	RoadVertexDesc v15_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(700, -50));
	RoadVertexDesc v16_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v16_desc] = v16;

	RoadVertex* v17 = new RoadVertex(QVector2D(800, -30));
	RoadVertexDesc v17_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v17_desc] = v17;

	RoadVertex* v18 = new RoadVertex(QVector2D(700, 250));
	RoadVertexDesc v18_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v18_desc] = v18;

	RoadVertex* v19 = new RoadVertex(QVector2D(850, 250));
	RoadVertexDesc v19_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v19_desc] = v19;

	RoadVertex* v20 = new RoadVertex(QVector2D(400, 100));
	RoadVertexDesc v20_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v20_desc] = v20;

	RoadVertex* v21 = new RoadVertex(QVector2D(650, 650));
	RoadVertexDesc v21_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v21_desc] = v21;

	RoadVertex* v22 = new RoadVertex(QVector2D(800, 700));
	RoadVertexDesc v22_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v22_desc] = v22;

	RoadVertex* v23 = new RoadVertex(QVector2D(600, 800));
	RoadVertexDesc v23_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v23_desc] = v23;

	RoadVertex* v24 = new RoadVertex(QVector2D(300, 550));
	RoadVertexDesc v24_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v24_desc] = v24;

	RoadVertex* v25 = new RoadVertex(QVector2D(0, 500));
	RoadVertexDesc v25_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v25_desc] = v25;

	RoadVertex* v26 = new RoadVertex(QVector2D(-200, 450));
	RoadVertexDesc v26_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v26_desc] = v26;

	RoadVertex* v27 = new RoadVertex(QVector2D(-350, 600));
	RoadVertexDesc v27_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v27_desc] = v27;

	RoadVertex* v28 = new RoadVertex(QVector2D(-400, 850));
	RoadVertexDesc v28_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v28_desc] = v28;

	RoadVertex* v29 = new RoadVertex(QVector2D(-150, 700));
	RoadVertexDesc v29_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v29_desc] = v29;

	RoadVertex* v30 = new RoadVertex(QVector2D(-250, 950));
	RoadVertexDesc v30_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v30_desc] = v30;

	RoadVertex* v31 = new RoadVertex(QVector2D(150, 800));
	RoadVertexDesc v31_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v31_desc] = v31;

	RoadVertex* v32 = new RoadVertex(QVector2D(50, 950));
	RoadVertexDesc v32_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v32_desc] = v32;

	RoadVertex* v33 = new RoadVertex(QVector2D(400, 910));
	RoadVertexDesc v33_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v33_desc] = v33;



	GraphUtil::addEdge(roads2, v0_desc, v2_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v1_desc, v2_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v2_desc, v3_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v2_desc, v26_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v3_desc, v4_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v3_desc, v5_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v3_desc, v25_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v5_desc, v6_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v5_desc, v20_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v5_desc, v25_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v6_desc, v7_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v6_desc, v10_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v6_desc, v14_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v7_desc, v8_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v7_desc, v9_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v10_desc, v11_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v12_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v13_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v15_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v11_desc, v16_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v14_desc, v15_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v15_desc, v16_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v15_desc, v20_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v16_desc, v17_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v16_desc, v18_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v18_desc, v19_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v18_desc, v20_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v18_desc, v21_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v20_desc, v24_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v21_desc, v22_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v21_desc, v23_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v21_desc, v24_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v24_desc, v25_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v24_desc, v31_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v25_desc, v26_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v25_desc, v29_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v26_desc, v27_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v27_desc, v28_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v27_desc, v29_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v29_desc, v30_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v29_desc, v31_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v31_desc, v32_desc, 1, 1, false);
	GraphUtil::addEdge(roads2, v31_desc, v33_desc, 1, 1, false);
}
