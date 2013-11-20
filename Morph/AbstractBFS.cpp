#include "AbstractBFS.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

AbstractBFS::AbstractBFS(const char* filename1, const char* filename2) {
	FILE* fp = fopen(filename1, "rb");
	roads1 = new RoadGraph();
	roads1->load(fp, 2);
	//GraphUtil::planarify(roads1);
	GraphUtil::singlify(roads1);
	GraphUtil::simplify(roads1, 30);
	fclose(fp);

	fp = fopen(filename2, "rb");
	roads2 = new RoadGraph();
	roads2->load(fp, 2);
	//GraphUtil::planarify(roads2);
	GraphUtil::singlify(roads2);
	GraphUtil::simplify(roads2, 30);
	fclose(fp);

	//createRoads1();
	//createRoads2();

	selected = 0;
}

AbstractBFS::~AbstractBFS() {
	delete roads1;
	delete roads2;
	clearSequence();
}

void AbstractBFS::draw(QPainter* painter, int offset, float scale) {
	if (roads1 == NULL) return;

	//drawGraph(painter, roads1, QColor(0, 0, 255), offset, scale, true);
	//drawGraph(painter, roads2, QColor(255, 0, 0), offset, scale, true);
	//drawRelation(painter, roads1, &correspondence, roads2, offset, scale);

	drawGraph(painter, sequence[selected], QColor(0, 0, 255), offset, scale);
}

void AbstractBFS::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale, bool label) {
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
		if (!v->valid) continue;

		int x = (v->getPt().x() + offset) * scale ;
		int y = (-v->getPt().y() + offset) * scale;
		painter->fillRect(x - 1, y - 1, 3, 3, col);

		if (label) {
			// 頂点番号をラベルとして表示する
			QString str;
			str.setNum(*vi);
			painter->drawText(x+2, y+13, str);
		}
	}
}

void AbstractBFS::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale) {
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

		int x1 = (v1->getPt().x() + offset) * scale;
		int y1 = (-v1->getPt().y() + offset) * scale;
		int x2 = (v2->getPt().x() + offset) * scale;
		int y2 = (-v2->getPt().y() + offset) * scale;
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
