#include "MTT.h"
#include "GraphUtil.h"
#include "Morph.h"
#include "PCA.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

MTT::MTT(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	roads = new RoadGraph();
	roads->load(fp, 2);
	GraphUtil::planarify(roads);
	GraphUtil::singlify(roads);
	GraphUtil::simplify(roads, 30, 0.0f);
	fclose(fp);

	selected = 0;
}

MTT::~MTT() {
	delete roads;
	clearSequence();
}

void MTT::draw(QPainter* painter, int offset, float scale) {
	if (roads == NULL) return;

	//drawGraph(painter, roads2, QColor(0, 0, 255), offset, scale);

	drawGraph(painter, sequence[selected], QColor(0, 0, 255), offset, scale);
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

void MTT::buildTree(float w1, float w2, float w3) {
	clearSequence();

	// 頂点の中で、degreeが1のものをcollapseしていく
	RoadGraph* r = GraphUtil::copyRoads(roads);
	collapse(r, w1, w2, w3, &sequence);
	delete r;

	return;

	/*
	// 生き残っている頂点を探す。
	std::list<RoadVertexDesc> v_list;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		v_list.push_back(*vi);
	}

	expand(roads);
	*/
}

/**
 * 頂点を、順番にcollapseしていく。
 * ただし、当該頂点から出るエッジの長さが短いものから、優先的にcollapseしていく。
 */
void MTT::collapse(RoadGraph* roads, float w1, float w2, float w3, std::vector<RoadGraph*>* sequence) {
	qDebug() << "collapse start.";

	PCA pca;
	doPCA(roads, pca);

	RoadVertexDesc bdry1_desc, bdry2_desc;
	findBoundaryVertices(roads, bdry1_desc, bdry2_desc);

	while (true) {
		int count = 0;

		sequence->push_back(GraphUtil::copyRoads(roads));

		float min_metric = std::numeric_limits<float>::max();
		RoadVertexDesc min_v_desc1;	// to be collapsed to the other one
		RoadVertexDesc min_v_desc2;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			count++;

			RoadVertexDesc src = boost::source(*ei, roads->graph);
			RoadVertexDesc tgt = boost::target(*ei, roads->graph);

			if (bdry2_desc == src || bdry2_desc == tgt) {
				int k = 0;
			}

			float metric = getMetric(roads, src, tgt, w1, w2, w3, pca);		
			if (metric < min_metric) {
				min_metric = metric;
				min_v_desc1 = src;
				min_v_desc2 = tgt;
			}

			metric = getMetric(roads, tgt, src, w1, w2, w3, pca);		
			if (metric < min_metric) {
				min_metric = metric;
				min_v_desc1 = tgt;
				min_v_desc2 = src;
			}
		}

		if (count <= 1) break;

		// 頂点min_v_desc1を、min_v_desc2へcollapseする
		GraphUtil::collapseVertex(roads, min_v_desc1, min_v_desc2);
	}

	qDebug() << "collapse done.";
}

/**
 * 頂点v1を、頂点v2にcollapseした場合の変化量(metric)を計算して返却する。
 */
float MTT::getMetric(RoadGraph* roads, RoadVertexDesc v1_desc, RoadVertexDesc v2_desc, float w1, float w2, float w3, PCA &pca) {
	// 距離の変化量
	float dist_old = 0.0f;
	float dist_new = 0.0f;
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v1_desc, roads->graph); ei != eend; ++ei) {
		RoadVertexDesc tgt_desc = boost::target(*ei, roads->graph);
		//dist_old += roads->graph[*ei]->getLength();
		dist_old += (roads->graph[v2_desc]->getPt() - roads->graph[v1_desc]->getPt()).length();

		if (tgt_desc == v2_desc) continue;

		dist_new += (roads->graph[tgt_desc]->getPt() - roads->graph[v2_desc]->getPt()).length();
	}
	float dist = fabs(dist_old - dist_new);

	// 角度の変化量（最も角度変化の大きいものを、変化量として採用する）
	float angle = 0.0f;
	QVector2D base_dir = roads->graph[v2_desc]->getPt() - roads->graph[v1_desc]->getPt();
	for (boost::tie(ei, eend) = boost::out_edges(v1_desc, roads->graph); ei != eend; ++ei) {
		RoadVertexDesc tgt_desc = boost::target(*ei, roads->graph);
		if (tgt_desc == v2_desc) continue;
		if (tgt_desc == v1_desc) continue;

		QVector2D dir = roads->graph[tgt_desc]->getPt() - roads->graph[v1_desc]->getPt();

		float a = GraphUtil::diffAngle(base_dir, dir);
		if (a > angle) {
			angle = a;
		}
	}

	// 縮み度
	int v1_id = GraphUtil::getVertexIndex(roads, v1_desc, false);

	float min_score1 = std::numeric_limits<float>::max();
	float max_score1 = -std::numeric_limits<float>::max();
	float min_score2 = std::numeric_limits<float>::max();
	float max_score2 = -std::numeric_limits<float>::max();
	int min_pt_id, max_pt_id;
	for (int i = 0; i < pca.score.rows; i++) {
		if (!roads->graph[GraphUtil::getVertex(roads, i, false)]->valid) continue;

		float score = pca.score.at<double>(i, 0);

		if (score < min_score1) {
			min_score1 = score;
		}
		if (score > max_score1) {
			max_score1 = score;
		}

		if (i == v1_id) continue;

		if (score < min_score2) {
			min_score2 = score;
		}
		if (score > max_score2) {
			max_score2 = score;
		}
	}
	float shrink = (max_score1 - min_score1) - (max_score2 - min_score2);

	return dist * w1 + angle * w2 + shrink * w3;
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
	}

	qDebug() << "expand done.";
}

void MTT::findBoundaryVertices(RoadGraph* roads, RoadVertexDesc &v1_desc, RoadVertexDesc &v2_desc) {
	PCA pca;
	doPCA(roads, pca);

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

void MTT::doPCA(RoadGraph* roads, PCA& pca) {
	// 頂点データを使って行列を生成する
	cv::Mat vmat = cv::Mat(GraphUtil::getNumVertices(roads), 2, CV_64FC1);

	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		vmat.at<double>(count, 0) = roads->graph[*vi]->getPt().x();
		vmat.at<double>(count, 1) = roads->graph[*vi]->getPt().y();

		count++;
	}

	// PCAを実施
	pca.pca(vmat, false);
}

void MTT::selectSequence(int selected) {
	this->selected = selected;
}

void MTT::clearSequence() {
	for (int i = 0; i < sequence.size(); i++) {
		sequence[i]->clear();
		delete sequence[i];
	}
	sequence.clear();
}