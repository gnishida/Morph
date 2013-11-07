#include "MMT.h"
#include "GraphUtil.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

VertexPriority::VertexPriority() {
	desc = -1;
	priority = 0;
}

VertexPriority::VertexPriority(RoadVertexDesc desc, float priority) {
	this->desc = desc;
	this->priority = priority;
}

MMT::MMT(Morph* morph, const char* filename) {
	this->morph = morph;

	FILE* fp = fopen(filename, "rb");
	roads = new RoadGraph();
	roads->load(fp, 2);
}

void MMT::draw(QPainter* painter, int offset, float scale) {
	if (roads == NULL) return;

	drawGraph(painter, roads, QColor(0, 0, 255), offset, scale);
}

void MMT::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
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

void MMT::buildTree() {
	// 頂点の中で、degreeが1のものをcollapseしていく
	collapse(roads);

	// 生き残っている頂点を探す。
	std::list<RoadVertexDesc> v_list;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		v_list.push_back(*vi);
	}

	expand(roads);
}

/**
 * 頂点を、順番にcollapseしていく。
 * ただし、当該頂点から出るエッジの長さが短いものから、優先的にcollapseしていく。
 */
void MMT::collapse(RoadGraph* roads) {
	qDebug() << "collapse start.";

	RoadOutEdgeIter oei, oeend;
	for (boost::tie(oei, oeend) = boost::out_edges(33, roads->graph); oei != oeend; ++oei) {
		RoadVertexDesc tgt = boost::target(*oei, roads->graph);
		int k = 0;
	}

	int count = 0;

	while (true) {
		if (count == 27) {
			int k = 0;
		}

		float min_len = std::numeric_limits<float>::max();
		RoadEdgeDesc min_e_desc;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			float len = roads->graph[*ei]->getLength();
			
			if (len < min_len) {
				min_len = len;
				min_e_desc = *ei;
			}
		}

		if (min_len == std::numeric_limits<float>::max()) break;

		GraphUtil::collapseEdge(roads, min_e_desc);

		// 再描画
		morph->update();

		qDebug() << "remove edge." << (++count);

		// 300ミリ秒待機
		QTest::qWait(300);
	}

	qDebug() << "collapse done.";
}

/**
 * Collapseした道路網を、親子関係の木構造を使って、元に戻す。
 */
void MMT::expand(RoadGraph* roads) {
	qDebug() << "expand start.";

	for (int i = roads->collapseHistory.size() - 1; i >= 0; i--) {
		RoadVertexDesc v1 = roads->collapseHistory[i].parentNode;
		RoadVertexDesc v2 = roads->collapseHistory[i].childNode;

		// 子ノードv2を有効にする
		roads->graph[v2]->valid = true;

		// v1とv2の間のエッジを有効にする
		RoadEdgeDesc e = GraphUtil::getEdge(roads, v1, v2, false);
		roads->graph[e]->valid = true;

		// 子ノードv2からのエッジを有効にする
		for (int j = 0; j < roads->collapseHistory[i].removedEdges.size(); j++) {
			RoadEdgeDesc e2 = roads->collapseHistory[i].removedEdges[j];

			roads->graph[e2]->valid = true;
		}

		// 親ノードvからのエッジを無効にする
		for (int j = 0; j < roads->collapseHistory[i].addedEdges.size(); j++) {
			RoadEdgeDesc e2 = roads->collapseHistory[i].addedEdges[j];

			roads->graph[e2]->valid = false;
		}

		// 再描画
		morph->update();

		// 300ミリ秒待機
		QTest::qWait(300);
	}

	qDebug() << "expand done.";
}