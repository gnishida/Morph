#include "BFSMulti.h"
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
	float edge_threshold = 300.0f;
	float snap_threshold = 900.0f;

	if (t == 1.0f) return GraphUtil::copyRoads(roads1);
	if (t == 0.0f) return GraphUtil::copyRoads(roads2);

	RoadGraph* new_roads = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		RoadVertexDesc v2 = correspondence[*vi];

		// 頂点を作成
		RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt() * t + roads2->graph[v2]->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
		new_roads->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		RoadVertexDesc v1 = boost::source(*ei, roads1->graph);
		RoadVertexDesc u1 = boost::target(*ei, roads1->graph);

		RoadVertexDesc v2 = correspondence[v1];
		RoadVertexDesc u2 = correspondence[u1];

		// エッジを作成
		RoadEdgeDesc e_desc = GraphUtil::addEdge(new_roads, conv[v1], conv[u1], roads1->graph[*ei]);
		if (GraphUtil::hasEdge(roads2, v2, u2)) {
			new_roads->graph[e_desc]->polyLine = GraphUtil::interpolateEdges(roads1, *ei, v1, roads2, GraphUtil::getEdge(roads2, v2, u2), v2, t);
		} else {
			new_roads->graph[e_desc]->polyLine.clear();
			new_roads->graph[e_desc]->addPoint(new_roads->graph[conv[v1]]->getPt());
			new_roads->graph[e_desc]->addPoint(new_roads->graph[conv[u1]]->getPt());
		}
	}

	// DeadEndの頂点について、エッジ長がthreshold以下なら頂点とそのエッジを削除する
	bool deleted = true;
	while (deleted) {
		deleted = false;
		for (boost::tie(vi, vend) = boost::vertices(new_roads->graph); vi != vend; ++vi) {
			if (!new_roads->graph[*vi]->valid) continue;

			if (GraphUtil::getDegree(new_roads, *vi) > 1) continue;

			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, new_roads->graph); ei != eend; ++ei) {
				if (!new_roads->graph[*ei]->valid) continue;

				RoadVertexDesc tgt = boost::target(*ei, new_roads->graph);

				if (new_roads->graph[*ei]->getLength() < edge_threshold) {
					new_roads->graph[*vi]->valid = false;
					new_roads->graph[*ei]->valid = false;
					deleted = true;
				}
			}
		}
	}

	for (boost::tie(vi, vend) = boost::vertices(new_roads->graph); vi != vend; ++vi) {
		if (!new_roads->graph[*vi]->valid) continue;

		if (GraphUtil::getDegree(new_roads, *vi) != 1) continue;

		// 当該頂点と接続されている唯一の頂点を取得
		RoadVertexDesc tgt;
		RoadEdgeDesc e_desc;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, new_roads->graph); ei != eend; ++ei) {
			if (!new_roads->graph[*ei]->valid) continue;

			tgt = boost::target(*ei, new_roads->graph);
			e_desc = *ei;
			break;
		}

		// 近接頂点を探す
		RoadVertexDesc nearest_desc;
		float min_dist = std::numeric_limits<float>::max();

		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(new_roads->graph); vi2 != vend2; ++vi2) {
			if (!new_roads->graph[*vi2]->valid) continue;
			if (*vi2 == tgt) continue;
			if (GraphUtil::getDegree(new_roads, *vi2) == 1) continue;

			float dist = (new_roads->graph[*vi2]->pt - new_roads->graph[*vi]->pt).length();
			if (dist < min_dist) {
				nearest_desc = *vi2;
				min_dist = dist;
			}
		}

		// 当該頂点と近接頂点との距離が、snap_threshold未満か？
		if ((new_roads->graph[*vi]->pt - new_roads->graph[nearest_desc]->pt).length() < snap_threshold) {
			// 一旦、古いエッジを、近接頂点にスナップするよう移動する
			GraphUtil::moveEdge(new_roads, e_desc, new_roads->graph[nearest_desc]->pt, new_roads->graph[tgt]->pt);

			// 新しいエッジを追加する
			GraphUtil::addEdge(new_roads, nearest_desc, tgt, new_roads->graph[e_desc]);

			// 古いエッジを無効にする
			new_roads->graph[e_desc]->valid = false;

			// 当該頂点を無効にする
			new_roads->graph[*vi]->valid = false;
		}
	}

	return new_roads;
}

void BFSMulti::init() {
	// 道路網のエッジのImportanceを計算する
	GraphUtil::computeImportanceOfEdges(roads1, 1.0f, 1.0f, 1.0f);
	GraphUtil::computeImportanceOfEdges(roads2, 1.0f, 1.0f, 1.0f);

	float min_dissimilarity = std::numeric_limits<float>::max();

	// シード
	QList<RoadVertexDesc> seeds1;
	QList<RoadVertexDesc> seeds2;

	srand(1234567);

	// Importance順に並べたエッジリストを取得
	QList<RoadEdgeDesc> edges1 = roads1->getOrderedEdgesByImportance();
	QList<RoadEdgeDesc> edges2 = roads2->getOrderedEdgesByImportance();

	bool updated = true;
	int iteration = 0;
	while (updated && !edges1.empty() && !edges2.empty()) {
		qDebug() << (iteration++);

		updated = false;

		float min_diff = std::numeric_limits<float>::max();
		RoadEdgeDesc e1;
		RoadEdgeDesc e2;

		// 道路網１のTop10 Importantエッジに対して
		int topN = std::min(10, edges1.size());
		for (int i = 0; i < topN; i++) {
			// 似ている対応エッジを道路網２から探す
			for (int j = 0; j < edges2.size(); j++) {
				float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[i], roads2, edges2[j], 1.0f, 1.0f, 1.0f, 1.0f);
				if (diff < min_diff) {
					min_diff = diff;
					e1 = edges1[i];
					e2 = edges2[j];
				}
			}
		}

		// 道路網２のTop10 Importantエッジに対して
		topN = std::min(10, edges2.size());
		for (int i = 0; i < topN; i++) {
			// 似ている対応エッジを道路網１ら探す
			for (int j = 0; j < edges1.size(); j++) {
				float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[j], roads2, edges2[i], 1.0f, 1.0f, 1.0f, 1.0f);
				if (diff < min_diff) {
					min_diff = diff;
					e1 = edges1[j];
					e2 = edges2[i];
				}
			}
		}

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

		// シード候補を追加
		seeds1.push_back(src1);
		seeds1.push_back(tgt1);
		seeds2.push_back(src2);
		seeds2.push_back(tgt2);
	
		// テンポラリの道路網を作成する。
		RoadGraph* temp1 = GraphUtil::copyRoads(roads1);
		RoadGraph* temp2 = GraphUtil::copyRoads(roads2);

		// シードを使ってマッチングを探し、非類似度を計算する
		QMap<RoadVertexDesc, RoadVertexDesc> map1;
		QMap<RoadVertexDesc, RoadVertexDesc> map2;
		float dissimilarity = computeDissimilarity(temp1, seeds1, temp2, seeds2, map1, map2);
		
		// 非類似度が最小なら、ベストマッチングとして更新
		if (dissimilarity < min_dissimilarity * 1.1f) {
			updated = true;
			min_dissimilarity = std::min(dissimilarity, min_dissimilarity);
		}

		// テンポラリの道路網を削除する
		delete temp1;
		delete temp2;

		// もし、前回のループより、非類似度が改善しないなら、シードを正式採用せずに、ループを終了する
		//if (!updated) break;

		// Importance順に並べたエッジリストから、使用したペアを削除
		for (int j = 0; j < edges1.size(); j++) {
			if (edges1[j] == e1) {
				edges1.removeAt(j);
				break;
			}
		}
		for (int j = 0; j < edges2.size(); j++) {
			if (edges2[j] == e2) {
				edges2.removeAt(j);
				break;
			}
		}

		qDebug() << "Roads1: " << src1 << "-" << tgt1 << " Roads2: " << src2 << "-" << tgt2;

	}

	// 最終的に決まったシードを使って、最終的な非類似度を計算
	QMap<RoadVertexDesc, RoadVertexDesc> map1;
	QMap<RoadVertexDesc, RoadVertexDesc> map2;
	float dissimilarity = computeDissimilarity(roads1, seeds1, roads2, seeds2, map1, map2);

	correspondence = map1;

	// 孤立した頂点を削除
	GraphUtil::removeIsolatedVertices(roads1);
	GraphUtil::removeIsolatedVertices(roads2);

	/*
	qDebug() << "Min dissimilarity: " << min_dissimilarity;
	qDebug() << "     connectivity: " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 1.0f, 0.0f, 0.0f, 0.0f);
	qDebug() << "     split       : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 1.0f, 0.0f, 0.0f);
	qDebug() << "     angle       : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 0.0f, 1.0f, 0.0f);
	qDebug() << "     distance    : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 0.0f, 0.0f, 1.0f);
	*/

	// シーケンスを生成
	clearSequence();
	for (int i = 0; i <= 20; i++) {
		float t = 1.0f - (float)i * 0.05f;

		sequence.push_back(interpolate(t));
	}
}

/**
 * ２つの道路網を、木構造を使ってマッチングさせる。
 */
void BFSMulti::findCorrespondence(RoadGraph* roads1, BFSForest* forest1, RoadGraph* roads2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2) {
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

			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}

		// 残り者の子ノードのマッチングを探す
		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPairByDirection(roads1, parent1, forest1, map1, roads2, parent2, forest2, map2, false, child1, child2)) break;

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
bool BFSMulti::findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	// 子リストを取得
	std::vector<RoadVertexDesc> children1 = forest1->getChildren(parent1);
	std::vector<RoadVertexDesc> children2 = forest2->getChildren(parent2);

	// エッジの角度が最もちかいペアをマッチさせる
	for (int i = 0; i < children1.size(); i++) {
		if (map1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		QVector2D dir1 = roads1->graph[children1[i]]->getPt() - roads1->graph[parent1]->getPt();
		float theta1 = atan2f(dir1.y(), dir1.x());
		for (int j = 0; j < children2.size(); j++) {
			if (map2.contains(children2[j])) continue;
			if (!roads2->graph[children2[j]]->valid) continue;

			QVector2D dir2 = roads2->graph[children2[j]]->getPt() - roads2->graph[parent2]->getPt();
			float theta2 = atan2f(dir2.y(), dir2.x());

			float angle = GraphUtil::diffAngle(theta1, theta2);
			if (angle < min_angle) {
				min_angle = angle;
				min_id1 = i;
				min_id2 = j;
			}
		}
	}
	
	// ベストペアが見つかったか、チェック
	if (min_angle < std::numeric_limits<float>::max()) {
		child1 = children1[min_id1];
		child2 = children2[min_id2];

		return true;
	}

	if (onlyUnpairedNode) return false;

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
		GraphUtil::addEdge(roads2, parent2, v_desc, roads1->graph[e1_desc]->lanes, roads1->graph[e1_desc]->type, roads1->graph[e1_desc]->oneWay);

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
		GraphUtil::addEdge(roads1, parent1, v_desc, roads2->graph[e2_desc]->lanes, roads2->graph[e2_desc]->type, roads2->graph[e2_desc]->oneWay);

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
	findCorrespondence(roads1, &forest1, roads2, &forest2, map1, map2);

	// 非類似度を計算
	float unsimilarity = GraphUtil::computeDissimilarity(roads1, map1, roads2, map2, 1.0f, 1.0f, 1.0f, 1.0f);

	return unsimilarity;
}
