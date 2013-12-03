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
	showRoads1 = true;
	showRoads2 = true;
	showInterpolation = true;
	colorByGroup = true;
}

AbstractBFS::~AbstractBFS() {
	delete roads1;
	delete roads2;
	clearSequence();
}

void AbstractBFS::setRoad1(const char* filename) {
	if (roads1 != NULL) delete roads1;

	roads1 = loadRoad(filename);
}

void AbstractBFS::setRoad2(const char* filename) {
	if (roads2 != NULL) delete roads2;

	roads2 = loadRoad(filename);
}

RoadGraph* AbstractBFS::loadRoad(const char* filename) {
	RoadGraph* roads = new RoadGraph();

	FILE* fp = fopen(filename, "rb");
	roads->load(fp, 2);
	fclose(fp);

	GraphUtil::removeIsolatedVertices(roads);
	GraphUtil::removeDuplicateEdges(roads);
	GraphUtil::clean(roads);

	//GraphUtil::singlify(roads2);
	GraphUtil::simplify(roads, 100);
	GraphUtil::reduce(roads);
	GraphUtil::removeIsolatedEdges(roads);

	return roads;
}

void AbstractBFS::draw(QPainter* painter) {
	if (showRoads1 && roads1 != NULL) {
		drawGraph(painter, roads1, 20, 50, false);
	}
	if (showRoads2 && roads2 != NULL) {
		drawGraph(painter, roads2, 20, 50, false);
	}
	if (showInterpolation && selected >= 0 && selected < sequence.size()) {
		drawGraph(painter, sequence[selected], 20, 50, false);
	}
}

void AbstractBFS::drawGraph(QPainter *painter, RoadGraph *roads, int line_width, int rect_size, bool label) {
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));

	// 描画色をセットアップ
	QColor group_col[9] = {
		QColor(255, 0, 0),
		QColor(0, 0, 255),
		QColor(0, 255, 0),
		QColor(255, 255, 0),
		QColor(255, 0, 255),
		QColor(0, 255, 255),
		QColor(128, 255, 128),
		QColor(255, 128, 255),
		QColor(255, 255, 128)
	};

	// ルートエッジ用の描画色は、もとの描画色を半分暗くしたもの
	QColor dark_col[9];
	for (int i = 0; i < 9; i++) {
		dark_col[i].setRedF(group_col[i].redF() * 0.5f);
		dark_col[i].setGreenF(group_col[i].greenF() * 0.5f);
		dark_col[i].setBlueF(group_col[i].blueF() * 0.5f);
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* edge = roads->graph[*ei];
		if (!edge->valid) continue;

		QColor col;
		if (!colorByGroup) {
			col = QColor(0, 0, 0);
		} else if (edge->seed) {
			col = dark_col[edge->group % 9];
		} else {
			col = group_col[edge->group % 9];
		}

		if (edge->seed) {
			painter->setPen(QPen(col, line_width * 2.0f, Qt::SolidLine, Qt::RoundCap));
		} else {
			painter->setPen(QPen(col, line_width, Qt::SolidLine, Qt::RoundCap));
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
	painter->setFont(font);

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		RoadVertex* v = roads->graph[*vi];
		if (!v->valid) continue;

		int x = v->getPt().x();
		int y = -v->getPt().y();
		painter->fillRect(x - rect_size/2, y - rect_size/2, rect_size, rect_size, QColor(0, 0, 0));

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
