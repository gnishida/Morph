#include "BFSProp.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

BFSProp::BFSProp() : AbstractBFS() {
	tree1 = NULL;
	tree2 = NULL;
}

BFSProp::~BFSProp() {
	delete tree1;
	delete tree2;
}

RoadGraph* BFSProp::interpolate(float t) {
	if (t == 1.0f) return GraphUtil::copyRoads(roads1);
	if (t == 0.0f) return GraphUtil::copyRoads(roads2);

	RoadGraph* roads = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// roads1の各頂点について
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		// 対応ノードを取得
		RoadVertexDesc v2 = correspondence[*vi];

		// Interpolationノードを作成
		RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt() * t + roads2->graph[v2]->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
		roads->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	// roads1の各エッジについて
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		RoadVertexDesc v1a = boost::source(*ei, roads1->graph);
		RoadVertexDesc v1b = boost::target(*ei, roads1->graph);

		RoadVertexDesc new_va = conv[v1a];
		RoadVertexDesc new_vb = conv[v1b];

		GraphUtil::addEdge(roads, new_va, new_vb, roads1->graph[*ei]->lanes, roads1->graph[*ei]->type, roads1->graph[*ei]->oneWay);
	}
	
	return roads;
}

void BFSProp::init() {
	// バネの原理で、normalizeする
	BBox area;
	area.addPoint(QVector2D(-5000, -5000));
	area.addPoint(QVector2D(5000, 5000));

	RoadGraph* normalized_roads1 = GraphUtil::copyRoads(roads1);
	RoadGraph* normalized_roads2 = GraphUtil::copyRoads(roads2);

	for (int i = 0; i < 100; i++) {
		// 道路網１を均等化
		GraphUtil::normalizeBySpring(normalized_roads1, area);
		//GraphUtil::getBoudingBox(normalized_roads1, -M_PI / 4.0f, M_PI / 4.0f);
		//GraphUtil::scaleToBBox(normalized_roads1, area);

		// 道路網２を均等化
		GraphUtil::normalizeBySpring(normalized_roads2, area);
	}

	// 道路網１を回転させながら、道路網２とマッチングさせ、その時の非類似性を計算し、最小となる組合せを見つける
	correspondence = findMinUnsimilarity(normalized_roads1, normalized_roads2);

	// 与えられた組合せを使って、残りのマッチングを見つける
	//correnspondence = findCorrespondence(roads1, roads2, correspondence);

	// シーケンスを生成
	clearSequence();
	for (int i = 0; i <= 20; i++) {
		float t = 1.0f - (float)i * 0.05f;

		sequence.push_back(interpolate(t));
	}
}

/**
 * 道路網１を回転させながら、道路網２とマッチングさせ、その時の非類似性を計算し、最小となる組合せを見つける。
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFSProp::findMinUnsimilarity(RoadGraph* roads1, RoadGraph* roads2) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	float min_diff = std::numeric_limits<float>::max();
	float min_theta, min_dx, min_dy;
	QMap<RoadVertexDesc, RoadVertexDesc> min_map1;
	QMap<RoadVertexDesc, RoadVertexDesc> min_map2;

	for (float theta = -M_PI / 2.0f; theta <= M_PI / 2.0f; theta += 0.17453f) {
		for (float dy = -1000.0f; dy <= 1000.0f; dy += 500.0f) {
			for (float dx = -1000.0f; dx <= 1000.0f; dx += 500.0f) {
				// 道路網１を回転・移動させる
				RoadGraph* transformed_roads1 = GraphUtil::copyRoads(roads1);
				GraphUtil::rotate(transformed_roads1, theta);
				GraphUtil::translate(transformed_roads1, QVector2D(dx, dy));

				// 道路網１と道路網２のマッピングを求める
				QMap<RoadVertexDesc, RoadVertexDesc> map1;
				QMap<RoadVertexDesc, RoadVertexDesc> map2;
				GraphUtil::findCorrespondenceByNearestNeighbor(roads1, roads2, map1, map2);

				// 道路網１と道路網２の非類似性を計算する。
				float diff = GraphUtil::computeDissimilarity(transformed_roads1, map1, roads2, map2, 1.0f, 1.0f, 1.0f, 1.0f);
				if (diff < min_diff) {
					min_diff = diff;
					min_theta = theta;
					min_dx = dx;
					min_dy = dy;
					min_map1 = map1;
					min_map2 = map2;
				}
			}
		}
	}

	// 重複マッチングがあれば、degreeが最も大きいものを残して、残りは削除
	QMap<RoadVertexDesc, RoadVertexDesc> refined_map1;
	QMap<RoadVertexDesc, RoadVertexDesc> refined_map2;

	QMap<RoadVertexDesc, int> max_degree2;
	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = min_map1.begin(); it != min_map1.end(); ++it) {
		RoadVertexDesc v1 = it.key();
		RoadVertexDesc v2 = it.value();

		int degree = GraphUtil::getDegree(roads1, v1);

		if (max_degree2.contains(v2)) {
			if (degree > max_degree2[v2]) {
				max_degree2[v2] = degree;

				refined_map1.remove(refined_map2[v2]);

				refined_map1[v1] = v2;
				refined_map2[v2] = v1;
			}
		} else {
			max_degree2[v2] = degree;
			refined_map1[v1] = v2;
			refined_map2[v2] = v1;
		}
	}

	return refined_map1;
}

/**
 * 与えられたマッチング情報からスタートし、その周辺ノードを探索しながら、
 * その他のノードもマッチングさせる。
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFSProp::findCorrespondence(RoadGraph* roads1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc> map) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	QList<RoadVertexDesc> queue1;	// 道路網１の頂点キュー
	QList<RoadVertexDesc> queue2;	// 道路網２の頂点キュー

	QList<RoadVertexDesc> visited1;
	QList<RoadVertexDesc> visited2;

	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = map.begin(); it != map.end(); ++it) {
		queue1.push_back(it.key());
		queue2.push_back(it.value());
	}

	while (!queue1.empty()) {
		RoadVertexDesc v1 = queue1.front();
		queue1.pop_front();
		RoadVertexDesc v2 = queue2.front();
		queue2.pop_front();

		// 道路網１で、当該頂点の隣接ノードリストを取得
		QList<RoadVertexDesc> children1;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v1, roads1->graph); ei != eend; ++ei) {
			if (!roads1->graph[*ei]->valid) continue;

			RoadVertexDesc u1 = boost::target(*ei, roads1->graph);
			if (visited1.contains(u1)) continue;
			if (queue1.contains(u1)) continue;

			children1.push_back(u1);
		}

		// 道路網２で、当該頂点の隣接ノードリストを取得
		QList<RoadVertexDesc> children2;
		for (boost::tie(ei, eend) = boost::out_edges(v2, roads2->graph); ei != eend; ++ei) {
			if (!roads2->graph[*ei]->valid) continue;

			RoadVertexDesc u2 = boost::target(*ei, roads2->graph);
			if (visited2.contains(u2)) continue;
			if (queue2.contains(u2)) continue;

			children2.push_back(u2);
		}

		/*
		QMap<RoadVertexDesc, RoadVertexDesc> pair = GraphUtil::findBestMatching(roads1, v1, children1, roads2, v2, children2);
		for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = pair.begin(); it != pair.end(); ++it) {

		}
		*/
	}

	return correspondence;
}
