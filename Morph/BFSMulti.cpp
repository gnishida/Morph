﻿#include "BFSMulti.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

BFSMulti::BFSMulti() : AbstractBFS() {
	forest1 = NULL;
	forest2 = NULL;
}

BFSMulti::~BFSMulti() {
	delete forest1;
	delete forest2;
}

RoadGraph* BFSMulti::interpolate(float t) {
	float deadend_removal_threshold = 600.0f;
	float snap_deadend_threshold = 600.0f;
	float min_angle_threshold = 0.3f;

	if (t == 1.0f) return GraphUtil::copyRoads(roads1);
	if (t == 0.0f) return GraphUtil::copyRoads(roads2);

	RoadGraph* new_roads = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// Add vertices to the interpolated roads
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		RoadVertexDesc v2 = correspondence[*vi];

		// Add a vertex
		RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt() * t + roads2->graph[v2]->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
		new_roads->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	// Add edges to the interpolated roads
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		RoadVertexDesc v1 = boost::source(*ei, roads1->graph);
		RoadVertexDesc u1 = boost::target(*ei, roads1->graph);

		RoadVertexDesc v2 = correspondence[v1];
		RoadVertexDesc u2 = correspondence[u1];

		// Is there a corresponding edge?
		if (GraphUtil::hasEdge(roads2, v2, u2)) {
			RoadEdgeDesc e2 = GraphUtil::getEdge(roads2, v2, u2);

			RoadEdgeDesc e_desc = GraphUtil::addEdge(new_roads, conv[v1], conv[u1], roads1->graph[*ei]);
			new_roads->graph[e_desc]->polyLine = GraphUtil::interpolateEdges(roads1, *ei, v1, roads2, e2, v2, t);

			// Remove the too short edge
			if (roads1->graph[*ei]->getLength() < 1.0f && !roads1->graph[*ei]->fullyPaired && t > 0.7f) {
				//GraphUtil::collapseEdge(new_roads, e_desc);
			}

			// Remove the too short edge
			if (roads2->graph[e2]->getLength() < 1.0f && !roads2->graph[e2]->fullyPaired && t < 0.3f) {
				//GraphUtil::collapseEdge(new_roads, e_desc);
			}
		}
	}
	
	// Snapping (for dead end edges)
	GraphUtil::snapDeadendEdges2(new_roads, 1, snap_deadend_threshold);

	// Remove the too short dead end edges
	GraphUtil::removeShortDeadend(new_roads, deadend_removal_threshold);

	// Snapping again (for dead end edges)
	GraphUtil::snapDeadendEdges2(new_roads, 1, snap_deadend_threshold);

	// Snapping (for other edges with degree 2)
	GraphUtil::snapDeadendEdges2(new_roads, 2, snap_deadend_threshold * 0.65f);

	// Remove all the isolated vertices
	GraphUtil::removeIsolatedVertices(new_roads);

	return new_roads;
}

void BFSMulti::init() {
	float edge_dissimilarity_threshold = 3.0f;

	// 道路網のエッジのImportanceを計算する
	GraphUtil::computeImportanceOfEdges(roads1, 1.0f, 1.0f, 1.0f);
	GraphUtil::computeImportanceOfEdges(roads2, 1.0f, 1.0f, 1.0f);

	QList<EdgePair> pairs = GraphUtil::getClosestEdgePairs(roads1, roads2, 40);
	QList<EdgePair> chosen_pairs;

	// シード
	QList<RoadVertexDesc> seeds1;
	QList<RoadVertexDesc> seeds2;

	srand(1234567);

	float min_dissimilarity = std::numeric_limits<float>::max();

	int iteration = 0;
	while (!pairs.empty()) {
		qDebug() << (iteration++);

		// 次のエッジペアを取得
		RoadEdgeDesc e1 = pairs.front().edge1;
		RoadEdgeDesc e2 = pairs.front().edge2;
		pairs.pop_front();

		float diff = GraphUtil::computeDissimilarityOfEdges(roads1, e1, roads2, e2);

		// 選択されたエッジペアの非類似度が閾値より大きい場合は、終了
		if (iteration > 1 && diff > edge_dissimilarity_threshold) break;

		// 選択されたペアから、両端のノードを取得
		RoadVertexDesc src1 = boost::source(e1, roads1->graph);
		RoadVertexDesc tgt1 = boost::target(e1, roads1->graph);
		RoadVertexDesc src2 = boost::source(e2, roads2->graph);
		RoadVertexDesc tgt2 = boost::target(e2, roads2->graph);

		// もしsrc1-tgt2、tgt1-src2の方が近かったら、src2とtgt2を入れ替える
		if ((roads1->graph[src1]->pt - roads2->graph[src2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[tgt2]->pt).length() > (roads1->graph[src1]->pt - roads2->graph[tgt2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[src2]->pt).length()) {
			src2 = boost::target(e2, roads2->graph);
			tgt2 = boost::source(e2, roads2->graph);
		}

		// copy the roads to the temporal ones
		RoadGraph* temp1 = GraphUtil::copyRoads(roads1);
		RoadGraph* temp2 = GraphUtil::copyRoads(roads2);

		// Add seeds
		seeds1.push_back(src1);
		seeds1.push_back(tgt1);
		seeds2.push_back(src2);
		seeds2.push_back(tgt2);

		// Compute the dissimilarity
		QMap<RoadVertexDesc, RoadVertexDesc> map1;
		QMap<RoadVertexDesc, RoadVertexDesc> map2;
		float dissimilarity = computeDissimilarity(temp1, seeds1, temp2, seeds2, map1, map2);
		
		// delete the temporal roads
		delete temp1;
		delete temp2;

		// If the dissiminarity becomes too bad, stop the iteration
		if (dissimilarity >= min_dissimilarity * 1.1f) {
			seeds1.pop_back();
			seeds1.pop_back();
			seeds2.pop_back();
			seeds2.pop_back();

			break;
		}

		chosen_pairs.push_back(EdgePair(e1, e2));

		// Update the best dissimilarity
		min_dissimilarity = std::min(dissimilarity, min_dissimilarity);

		qDebug() << "Roads1: " << src1 << "-" << tgt1 << " Roads2: " << src2 << "-" << tgt2 << " (Dissimilarity: " << min_dissimilarity << ")";
	}

	// Build forests by using the finalized seeds
	BFSForest forest1(roads1, seeds1);
	BFSForest forest2(roads2, seeds2);

	// Find the final matching by using the forests
	QMap<RoadVertexDesc, RoadVertexDesc> map1;
	QMap<RoadVertexDesc, RoadVertexDesc> map2;
	findCorrespondence(roads1, &forest1, roads2, &forest2, true, map1, map2);

	correspondence = map1;

	// Remove the isolated vertices
	GraphUtil::removeIsolatedVertices(roads1);
	GraphUtil::removeIsolatedVertices(roads2);

	// Apply Rigid Iterative Closest Point (ICP)
	//GraphUtil::rigidICP(roads1, roads2, pairs);

	// Generate sequence of interpolated roads
	clearSequence();
	for (int i = 0; i <= 20; i++) {
		float t = 1.0f - (float)i * 0.05f;

		sequence.push_back(interpolate(t));
	}
}

/**
 * ２つの道路網を、木構造を使ってマッチングさせる。
 * completeMatchingフラグがtrueなら、対応がない子ノードも、無理やり対応相手を作って対応させる。
 */
void BFSMulti::findCorrespondence(RoadGraph* roads1, BFSForest* forest1, RoadGraph* roads2, BFSForest* forest2, bool completeMatching, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2) {
	std::list<RoadVertexDesc> seeds1;
	std::list<RoadVertexDesc> seeds2;

	// 各ルートエッジについて
	for (int i = 0; i < forest1->getRoots().size(); i++) {
		RoadVertexDesc v1 = forest1->getRoots()[i];
		RoadVertexDesc v2 = forest2->getRoots()[i];

		// ルート頂点をマッチングさせる
		map1[v1] = v2;
		map2[v2] = v1;

		// ルート頂点を、シードに追加する
		seeds1.push_back(v1);
		seeds2.push_back(v2);
	}

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (forest1->getChildren(parent1).size() == 0 && forest2->getChildren(parent2).size() == 0) continue;

		// 子リストを取得
		std::vector<RoadVertexDesc> children1 = forest1->getChildren(parent1);
		std::vector<RoadVertexDesc> children2 = forest2->getChildren(parent2);

		// 子ノード同士のベストマッチングを取得
		QMap<RoadVertexDesc, RoadVertexDesc> children_map = GraphUtil::findCorrespondentEdges(roads1, parent1, children1, roads2, parent2, children2);
		for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = children_map.begin(); it != children_map.end(); ++it) {
			RoadVertexDesc child1 = it.key();
			RoadVertexDesc child2 = it.value();

			// マッチングを更新
			map1[child1] = child2;
			map2[child2] = child1;

			// fullyPairedフラグをtrueにする
			roads1->graph[GraphUtil::getEdge(roads1, parent1, child1)]->fullyPaired = true;
			roads2->graph[GraphUtil::getEdge(roads2, parent2, child2)]->fullyPaired = true;

			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}

		if (!completeMatching) continue;

		// 残り者の子ノードのマッチングを探す
		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPairByDirection(roads1, parent1, forest1, map1, roads2, parent2, forest2, map2, child1, child2)) break;

			// マッチングを更新
			map1[child1] = child2;
			map2[child2] = child1;

			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}
	}
}

/**
 * 子ノードリスト１と子ノードリスト２から、ベストペアを探し出す。
 * まずは、ペアになっていないノードから候補を探す。
 * 既に、一方のリストが全てペアになっている場合は、当該リストからは、ペアとなっているものも含めて、ベストペアを探す。ただし、その場合は、ペアとなったノードをコピーして、必ず１対１ペアとなるようにする。
 */
bool BFSMulti::findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	// 子リストを取得
	std::vector<RoadVertexDesc> children1 = forest1->getChildren(parent1);
	std::vector<RoadVertexDesc> children2 = forest2->getChildren(parent2);

	// ベストペアが見つからない、つまり、一方のリストが、全てペアになっている場合
	for (int i = 0; i < children1.size(); i++) {
		if (map1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		// 相手の親ノードをコピーしてマッチさせる
		RoadVertex* v = new RoadVertex(roads2->graph[parent2]->getPt());
		RoadVertexDesc v_desc = boost::add_vertex(roads2->graph);
		roads2->graph[v_desc] = v;

		RoadEdgeDesc e1_desc = GraphUtil::getEdge(roads1, parent1, children1[i]);

		// 相手の親ノードと子ノードの間にエッジを作成する
		//RoadEdgeDesc e2_desc = GraphUtil::addEdge(roads2, parent2, v_desc, roads1->graph[e1_desc]->lanes, roads1->graph[e1_desc]->type, roads1->graph[e1_desc]->oneWay);
		RoadEdgeDesc e2_desc = GraphUtil::addEdge(roads2, parent2, v_desc, roads1->graph[e1_desc]);
		roads2->graph[e2_desc]->polyLine.clear();
		roads2->graph[e2_desc]->addPoint(roads2->graph[parent2]->pt);
		roads2->graph[e2_desc]->addPoint(roads2->graph[v_desc]->pt);

		forest2->addChild(parent2, v_desc);

		child1 = children1[i];
		child2 = v_desc;

		return true;
	}

	for (int i = 0; i < children2.size(); i++) {
		if (map2.contains(children2[i])) continue;
		if (!roads2->graph[children2[i]]->valid) continue;

		// 相手の親ノードをコピーしてマッチさせる
		RoadVertex* v = new RoadVertex(roads1->graph[parent1]->getPt());
		RoadVertexDesc v_desc = boost::add_vertex(roads1->graph);
		roads1->graph[v_desc] = v;

		RoadEdgeDesc e2_desc = GraphUtil::getEdge(roads2, parent2, children2[i]);

		// 相手の親ノードと子ノードの間にエッジを作成する
		//GraphUtil::addEdge(roads1, parent1, v_desc, roads2->graph[e2_desc]->lanes, roads2->graph[e2_desc]->type, roads2->graph[e2_desc]->oneWay);
		RoadEdgeDesc e1_desc = GraphUtil::addEdge(roads1, parent1, v_desc, roads2->graph[e2_desc]);
		roads1->graph[e1_desc]->polyLine.clear();
		roads1->graph[e1_desc]->addPoint(roads1->graph[parent1]->pt);
		roads1->graph[e1_desc]->addPoint(roads1->graph[v_desc]->pt);

		forest1->addChild(parent1, v_desc);

		child1 = v_desc;
		child2 = children2[i];

		return true;
	}

	// ペアなし、つまり、全ての子ノードがペアになっている
	return false;
}

/**
 * 与えられたシードを使ってフォレストを作成し、２つの道路網の類似性を計算する。
 */
float BFSMulti::computeDissimilarity(RoadGraph* roads1, QList<RoadVertexDesc> seeds1, RoadGraph* roads2, QList<RoadVertexDesc> seeds2, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2) {
	// シードを使ってフォレストを構築
	BFSForest forest1(roads1, seeds1);
	BFSForest forest2(roads2, seeds2);

	// フォレストを使って、マッチングを探す
	findCorrespondence(roads1, &forest1, roads2, &forest2, false, map1, map2);

	// 非類似度を計算
	float unsimilarity = GraphUtil::computeDissimilarity2(roads1, map1, roads2, map2, 1.0f, 1.0f, 1.0f, 1.0f);

	return unsimilarity;
}
