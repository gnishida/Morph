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
	fclose(fp);

	GraphUtil::removeDuplicateEdges(roads1);
	GraphUtil::clean(roads1);

	//GraphUtil::singlify(roads1);			// Canberraなど、singlifyしない方が良いと思われる
	GraphUtil::simplify(roads1, 100);
	GraphUtil::removeDeadEnd(roads1);		// やはり、deadEndを残すことにする。
	GraphUtil::reduce(roads1);

	GraphUtil::clean(roads1);
	GraphUtil::planarify(roads1);

	// 道路のヒストグラム情報を出力
	//GraphUtil::printStatistics(roads1);

	if (roads2 != NULL) {
		init();
	} else {
		clearSequence();
		sequence.push_back(GraphUtil::copyRoads(roads1));
	}
}

void AbstractBFS::setRoad2(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	roads2 = new RoadGraph();
	roads2->load(fp, 2);
	fclose(fp);

	GraphUtil::removeDuplicateEdges(roads2);
	GraphUtil::clean(roads2);

	//GraphUtil::singlify(roads2);
	GraphUtil::simplify(roads2, 100);
	GraphUtil::removeDeadEnd(roads2);
	GraphUtil::reduce(roads2);

	GraphUtil::clean(roads2);
	GraphUtil::planarify(roads2);

	// 道路のヒストグラム情報を出力
	//GraphUtil::printStatistics(roads2);

	if (roads1 != NULL) {
		init();
	} else {
		clearSequence();
		sequence.push_back(GraphUtil::copyRoads(roads2));
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

RoadGraph* AbstractBFS::getSelectedRoads() {
	if (sequence.size() == 0 || selected < 0 || selected >= sequence.size()) return NULL;

	return sequence[selected];
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

void AbstractBFS::createRoads5() {
	roads1 = new RoadGraph();

	RoadVertex* v0 = new RoadVertex(QVector2D(-3000, 2500));
	RoadVertexDesc v0_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v0_desc] = v0;

	RoadVertex* v1 = new RoadVertex(QVector2D(-2000, 1200));
	RoadVertexDesc v1_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-4200, 500));
	RoadVertexDesc v2_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-3000, 0));
	RoadVertexDesc v3_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-3500, -1000));
	RoadVertexDesc v4_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(-2000, -500));
	RoadVertexDesc v5_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(400, 1900));
	RoadVertexDesc v6_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(100, 4600));
	RoadVertexDesc v7_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(900, 3300));
	RoadVertexDesc v8_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(3000, 4600));
	RoadVertexDesc v9_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(1800, 2000));
	RoadVertexDesc v10_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(3800, 2500));
	RoadVertexDesc v11_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(4500, 2500));
	RoadVertexDesc v12_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(-100, 500));
	RoadVertexDesc v13_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(3200, 1000));
	RoadVertexDesc v14_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(4400, 100));
	RoadVertexDesc v15_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(1400, -200));
	RoadVertexDesc v16_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v16_desc] = v16;

	RoadVertex* v17 = new RoadVertex(QVector2D(2900, -1100));
	RoadVertexDesc v17_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v17_desc] = v17;

	RoadVertex* v18 = new RoadVertex(QVector2D(4200, -1500));
	RoadVertexDesc v18_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v18_desc] = v18;

	RoadVertex* v19 = new RoadVertex(QVector2D(1500, -2000));
	RoadVertexDesc v19_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v19_desc] = v19;

	RoadVertex* v20 = new RoadVertex(QVector2D(100, -3500));
	RoadVertexDesc v20_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v20_desc] = v20;

	RoadVertex* v21 = new RoadVertex(QVector2D(2500, -3800));
	RoadVertexDesc v21_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v21_desc] = v21;

	RoadVertex* v22 = new RoadVertex(QVector2D(-1500, -1500));
	RoadVertexDesc v22_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v22_desc] = v22;

	RoadVertex* v23 = new RoadVertex(QVector2D(-3500, -2000));
	RoadVertexDesc v23_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v23_desc] = v23;

	RoadVertex* v24 = new RoadVertex(QVector2D(-1500, -3500));
	RoadVertexDesc v24_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v24_desc] = v24;

	RoadVertex* v25 = new RoadVertex(QVector2D(-800, -300));
	RoadVertexDesc v25_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v25_desc] = v25;

	RoadVertex* v26 = new RoadVertex(QVector2D(100, -800));
	RoadVertexDesc v26_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v26_desc] = v26;

	RoadVertex* v27 = new RoadVertex(QVector2D(-700, 3700));
	RoadVertexDesc v27_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v27_desc] = v27;

	GraphUtil::addEdge(roads1, v0_desc, v1_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v1_desc, v3_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v1_desc, v6_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v2_desc, v3_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v3_desc, v4_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v3_desc, v5_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v6_desc, v8_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v6_desc, v10_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v6_desc, v13_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v7_desc, v8_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v8_desc, v9_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v8_desc, v27_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v10_desc, v11_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v10_desc, v14_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v11_desc, v12_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v11_desc, v14_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v13_desc, v16_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v13_desc, v25_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v14_desc, v15_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v14_desc, v16_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v16_desc, v17_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v17_desc, v18_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v17_desc, v19_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v19_desc, v20_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v19_desc, v21_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v22_desc, v23_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v22_desc, v24_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v22_desc, v25_desc, 2, 2, false);
	GraphUtil::addEdge(roads1, v25_desc, v26_desc, 2, 2, false);
}

void AbstractBFS::createRoads6() {
	roads2 = new RoadGraph();

	RoadVertex* v0 = new RoadVertex(QVector2D(-3500, 3000));
	RoadVertexDesc v0_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v0_desc] = v0;

	RoadVertex* v1 = new RoadVertex(QVector2D(-3100, 3800));
	RoadVertexDesc v1_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-1800, 4900));
	RoadVertexDesc v2_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-2000, 3600));
	RoadVertexDesc v3_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-2500, 1200));
	RoadVertexDesc v4_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(-1200, 1500));
	RoadVertexDesc v5_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(0, 1800));
	RoadVertexDesc v6_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(-800, 4000));
	RoadVertexDesc v7_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(500, 1800));
	RoadVertexDesc v8_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(800, 3900));
	RoadVertexDesc v9_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(1600, 2100));
	RoadVertexDesc v10_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(3000, 2500));
	RoadVertexDesc v11_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(3600, 2600));
	RoadVertexDesc v12_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(0, 1200));
	RoadVertexDesc v13_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(1700, 1200));
	RoadVertexDesc v14_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(3000, 1000));
	RoadVertexDesc v15_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(3700, 1200));
	RoadVertexDesc v16_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v16_desc] = v16;

	RoadVertex* v17 = new RoadVertex(QVector2D(3600, 800));
	RoadVertexDesc v17_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v17_desc] = v17;

	RoadVertex* v18 = new RoadVertex(QVector2D(4000, 500));
	RoadVertexDesc v18_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v18_desc] = v18;

	RoadVertex* v19 = new RoadVertex(QVector2D(0, -500));
	RoadVertexDesc v19_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v19_desc] = v19;

	RoadVertex* v20 = new RoadVertex(QVector2D(1600, -500));
	RoadVertexDesc v20_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v20_desc] = v20;

	RoadVertex* v21 = new RoadVertex(QVector2D(3000, -500));
	RoadVertexDesc v21_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v21_desc] = v21;

	RoadVertex* v22 = new RoadVertex(QVector2D(4900, -200));
	RoadVertexDesc v22_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v22_desc] = v22;

	RoadVertex* v23 = new RoadVertex(QVector2D(0, -1800));
	RoadVertexDesc v23_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v23_desc] = v23;

	RoadVertex* v24 = new RoadVertex(QVector2D(0, -2500));
	RoadVertexDesc v24_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v24_desc] = v24;

	RoadVertex* v25 = new RoadVertex(QVector2D(-1000, -2100));
	RoadVertexDesc v25_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v25_desc] = v25;

	RoadVertex* v26 = new RoadVertex(QVector2D(-2000, -2400));
	RoadVertexDesc v26_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v26_desc] = v26;

	RoadVertex* v27 = new RoadVertex(QVector2D(-1800, -1500));
	RoadVertexDesc v27_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v27_desc] = v27;

	RoadVertex* v28 = new RoadVertex(QVector2D(-3000, -2500));
	RoadVertexDesc v28_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v28_desc] = v28;

	RoadVertex* v29 = new RoadVertex(QVector2D(-3400, -2200));
	RoadVertexDesc v29_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v29_desc] = v29;

	RoadVertex* v30 = new RoadVertex(QVector2D(-1900, -1100));
	RoadVertexDesc v30_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v30_desc] = v30;

	RoadVertex* v31 = new RoadVertex(QVector2D(-4900, -3200));
	RoadVertexDesc v31_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v31_desc] = v31;

	RoadVertex* v32 = new RoadVertex(QVector2D(-4000, -1100));
	RoadVertexDesc v32_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v32_desc] = v32;

	RoadVertex* v33 = new RoadVertex(QVector2D(-4900, -1100));
	RoadVertexDesc v33_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v33_desc] = v33;

	RoadVertex* v34 = new RoadVertex(QVector2D(-2500, -4900));
	RoadVertexDesc v34_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v34_desc] = v34;

	RoadVertex* v35 = new RoadVertex(QVector2D(1800, -1800));
	RoadVertexDesc v35_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v35_desc] = v35;

	RoadVertex* v36 = new RoadVertex(QVector2D(2500, -1400));
	RoadVertexDesc v36_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v36_desc] = v36;

	RoadVertex* v37 = new RoadVertex(QVector2D(1800, -2500));
	RoadVertexDesc v37_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v37_desc] = v37;

	RoadVertex* v38 = new RoadVertex(QVector2D(3100, -1700));
	RoadVertexDesc v38_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v38_desc] = v38;

	RoadVertex* v39 = new RoadVertex(QVector2D(4900, -2200));
	RoadVertexDesc v39_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v39_desc] = v39;

	RoadVertex* v40 = new RoadVertex(QVector2D(3300, -2800));
	RoadVertexDesc v40_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v40_desc] = v40;

	RoadVertex* v41 = new RoadVertex(QVector2D(4900, -3000));
	RoadVertexDesc v41_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v41_desc] = v41;

	RoadVertex* v42 = new RoadVertex(QVector2D(1800, -3500));
	RoadVertexDesc v42_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v42_desc] = v42;

	RoadVertex* v43 = new RoadVertex(QVector2D(1500, -3100));
	RoadVertexDesc v43_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v43_desc] = v43;

	RoadVertex* v44 = new RoadVertex(QVector2D(2000, -4900));
	RoadVertexDesc v44_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v44_desc] = v44;

	RoadVertex* v45 = new RoadVertex(QVector2D(-4000, 0));
	RoadVertexDesc v45_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v45_desc] = v45;

	RoadVertex* v46 = new RoadVertex(QVector2D(-3000, 2700));
	RoadVertexDesc v46_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v46_desc] = v46;

	GraphUtil::addEdge(roads2, v0_desc, v1_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v1_desc, v3_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v1_desc, v46_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v2_desc, v3_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v3_desc, v5_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v4_desc, v5_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v5_desc, v6_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v6_desc, v8_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v6_desc, v13_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v7_desc, v8_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v8_desc, v10_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v9_desc, v10_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v10_desc, v11_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v10_desc, v14_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v11_desc, v12_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v11_desc, v15_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v13_desc, v14_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v13_desc, v19_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v14_desc, v15_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v14_desc, v20_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v15_desc, v17_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v16_desc, v17_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v17_desc, v18_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v17_desc, v21_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v19_desc, v20_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v19_desc, v23_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v20_desc, v21_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v20_desc, v35_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v21_desc, v22_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v21_desc, v36_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v23_desc, v24_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v23_desc, v25_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v25_desc, v26_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v25_desc, v27_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v27_desc, v28_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v28_desc, v29_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v28_desc, v34_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v29_desc, v30_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v29_desc, v31_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v29_desc, v32_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v32_desc, v33_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v32_desc, v45_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v36_desc, v37_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v36_desc, v38_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v38_desc, v39_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v38_desc, v40_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v40_desc, v41_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v40_desc, v42_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v42_desc, v43_desc, 2, 2, false);
	GraphUtil::addEdge(roads2, v42_desc, v44_desc, 2, 2, false);
}
