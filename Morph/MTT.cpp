#include "MTT.h"
#include "GraphUtil.h"
#include "Morph.h"
#include "PCA.h"
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

MTT::MTT(Morph* morph, const char* filename1, const char* filename2) {
	this->morph = morph;

	FILE* fp = fopen(filename1, "rb");
	roads1 = new RoadGraph();
	roads1->load(fp, 2);
	GraphUtil::planarify(roads1);
	GraphUtil::singlify(roads1);
	GraphUtil::simplify(roads1, 30, 0.0f);
	fclose(fp);

	fp = fopen(filename2, "rb");
	roads2 = new RoadGraph();
	roads2->load(fp, 2);
	GraphUtil::planarify(roads2);
	GraphUtil::singlify(roads2);
	GraphUtil::simplify(roads2, 30, 0.0f);
	fclose(fp);
}

void MTT::draw(QPainter* painter, float t, int offset, float scale) {
	if (roads1 == NULL) return;

	//drawGraph(painter, roads2, QColor(0, 0, 255), offset, scale);

	int index = sequence2.size() * (1.0f - t);
	if (index >= sequence2.size()) index = sequence2.size() - 1;
	RoadGraph* interpolated = sequence2[index];
	drawGraph(painter, interpolated, QColor(0, 0, 255), offset, scale);
}

void MTT::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
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

void MTT::buildTree() {
	// 頂点の中で、degreeが1のものをcollapseしていく
	collapse(roads2, &sequence2);

	return;

	// 生き残っている頂点を探す。
	std::list<RoadVertexDesc> v_list;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		v_list.push_back(*vi);
	}

	expand(roads1);
}

/**
 * 頂点を、順番にcollapseしていく。
 * ただし、当該頂点から出るエッジの長さが短いものから、優先的にcollapseしていく。
 */
void MTT::collapse(RoadGraph* roads, std::vector<RoadGraph*>* sequence) {
	qDebug() << "collapse start.";

	RoadVertexDesc bdry1_desc, bdry2_desc;
	findBoundaryVertices(roads, bdry1_desc, bdry2_desc);

	int count = 0;

	while (true) {
		sequence->push_back(GraphUtil::copyRoads(roads));

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

		// エッジの両端の頂点を取得する
		RoadVertexDesc v1_desc = boost::source(min_e_desc, roads->graph);
		RoadVertexDesc v2_desc = boost::target(min_e_desc, roads->graph);

		// 頂点をcollapseする
		if (v1_desc == bdry1_desc || v1_desc == bdry2_desc) {
			GraphUtil::collapseVertex(roads, v2_desc, v1_desc);
		} else if (v2_desc == bdry1_desc || v2_desc == bdry2_desc) {
			GraphUtil::collapseVertex(roads, v1_desc, v2_desc);
		} else if (boost::degree(v1_desc, roads->graph) > boost::degree(v2_desc, roads->graph)) {
			GraphUtil::collapseVertex(roads, v2_desc, v1_desc);
		} else {
			GraphUtil::collapseVertex(roads, v1_desc, v2_desc);
		}

		/*
		// 再描画
		morph->update();

		qDebug() << "remove edge." << (++count);

		// 300ミリ秒待機
		QTest::qWait(300);
		*/
	}

	qDebug() << "collapse done.";
}

/**
 * Collapseした道路網を、親子関係の木構造を使って、元に戻す。
 */
void MTT::expand(RoadGraph* roads) {
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

void MTT::findBoundaryVertices(RoadGraph* roads, RoadVertexDesc &v1_desc, RoadVertexDesc &v2_desc) {
	// create matrix of vertex data
	cv::Mat vmat;
	createVertexMatrix(roads, vmat);

	PCA pca;
	pca.pca(vmat, false);

	QVector2D evector;
	evector.setX(pca.evectors.at<double>(0, 0));
	evector.setY(pca.evectors.at<double>(1, 0));

	float min_score = std::numeric_limits<float>::max();
	float max_score = -std::numeric_limits<float>::max();
	int min_pt_id, max_pt_id;
	for (int i = 0; i < pca.score.rows; i++) {
		float score = pca.score.at<double>(i, 0);
		if (score < min_score) {
			min_score = score;
			min_pt_id = i;
		}
		if (score > max_score) {
			max_score = score;
			max_pt_id = i;
		}
	}

	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		if (count == min_pt_id) {
			v1_desc = *vi;
		}
		if (count == max_pt_id) {
			v2_desc = * vi;
		}

		count++;
	}
}

void MTT::createVertexMatrix(RoadGraph* roads, cv::Mat& vmat) {
	vmat = cv::Mat(GraphUtil::getNumVertices(roads), 2, CV_64FC1);

	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		vmat.at<double>(count, 0) = roads->graph[*vi]->getPt().x();
		vmat.at<double>(count, 1) = roads->graph[*vi]->getPt().y();

		count++;
	}
}
