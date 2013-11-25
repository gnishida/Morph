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
	float snap_threshold = 1000.0f;

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
		/*
		RoadEdge* new_e = new RoadEdge(roads1->graph[*ei]->lanes, roads1->graph[*ei]->type, roads1->graph[*ei]->oneWay);
		if (GraphUtil::hasEdge(roads2, conv[v1], conv[u1])) {
			new_e->polyLine = GraphUtil::interpolateEdges(roads1->graph[*ei]->polyLine, roads2->graph[GraphUtil::getEdge(roads2, conv[v1], conv[u1])]->polyLine, t);
		} else {
			new_e->addPoint(new_roads->graph[conv[v1]]->getPt());
			new_e->addPoint(new_roads->graph[conv[u1]]->getPt());
		}
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(conv[v1], conv[u1], new_roads->graph);
		new_roads->graph[edge_pair.first] = new_e;
		*/
		
		GraphUtil::addEdge(new_roads, conv[v1], conv[u1], roads1->graph[*ei]->lanes, roads1->graph[*ei]->type, roads1->graph[*ei]->oneWay);
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

	// 一旦、無効頂点・エッジを削除して、きれいにする
	RoadGraph* temp = GraphUtil::clean(new_roads);
	delete new_roads;
	new_roads = temp;

	// DeadEndの頂点について、近くの他の頂点にSnapさせる
	for (boost::tie(vi, vend) = boost::vertices(new_roads->graph); vi != vend; ++vi) {
		if (GraphUtil::getDegree(new_roads, *vi) == 1) {
			new_roads->graph[*vi]->valid = false;
		}
	}
	for (boost::tie(vi, vend) = boost::vertices(new_roads->graph); vi != vend; ++vi) {
		if (new_roads->graph[*vi]->valid) continue;

		RoadVertexDesc nearest_v = GraphUtil::findNearestVertex(new_roads, new_roads->graph[*vi]->pt);

		if ((new_roads->graph[*vi]->pt - new_roads->graph[nearest_v]->pt).length() < snap_threshold) {
			// 当該頂点を、近接頂点にスナップする
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, new_roads->graph); ei != eend; ++ei) {
				RoadVertexDesc tgt = boost::target(*ei, new_roads->graph);

				// 古いエッジを無効にする
				new_roads->graph[*ei]->valid = false;

				// 新しいエッジを追加する
				GraphUtil::addEdge(new_roads, tgt, nearest_v, new_roads->graph[*ei]->lanes, new_roads->graph[*ei]->type, new_roads->graph[*ei]->oneWay);
			}
		} else {
			// 当該頂点を、有効に戻す
			new_roads->graph[*vi]->valid = true;
		}
	}

	return new_roads;
}

void BFSMulti::init() {
	// 道路網のウェイトを計算する
	roads1->computeEdgeWeights();
	roads2->computeEdgeWeights();

	float min_unsimilarity = std::numeric_limits<float>::max();
	QMap<RoadVertexDesc, RoadVertexDesc> min_map1;
	QMap<RoadVertexDesc, RoadVertexDesc> min_map2;
	RoadGraph* min_roads1 = NULL;
	RoadGraph* min_roads2 = NULL;

	srand(1234567);

	// シード
	std::vector<RoadVertexDesc> seeds1;
	std::vector<RoadVertexDesc> seeds2;

	// Importance順に並べたエッジリストを取得
	QList<RoadEdgeDesc> edges1 = roads1->getOrderedEdgesByImportance();
	QList<RoadEdgeDesc> edges2 = roads2->getOrderedEdgesByImportance();


	for (int i = 0; i < 1000; i++) {
		qDebug() << i;

		// エッジのペアを取得
		RoadEdgeDesc e1;
		RoadEdgeDesc e2;
		getSimilarEdges(roads1, roads2, edges1, edges2, e1, e2);




		// テンポラリのシードを作成
		std::vector<RoadVertexDesc> temp_seeds1 = seeds1;
		std::vector<RoadVertexDesc> temp_seeds2 = seeds2;

		RoadVertexDesc src1 = boost::source(e1, roads1->graph);
		RoadVertexDesc tgt1 = boost::target(e1, roads1->graph);
		RoadVertexDesc src2 = boost::source(e2, roads2->graph);
		RoadVertexDesc tgt2 = boost::target(e2, roads2->graph);

		// テンポラリシードに、シード候補を追加
		temp_seeds1.push_back(src1);
		temp_seeds1.push_back(tgt1);
		temp_seeds2.push_back(src2);
		temp_seeds2.push_back(tgt2);

		// テンポラリの道路網を作成		
		RoadGraph* temp1 = GraphUtil::copyRoads(roads1);
		RoadGraph* temp2 = GraphUtil::copyRoads(roads2);

		// テンポラリのシードを使ってフォレストを構築
		BFSForest forest1(temp1, temp_seeds1);
		BFSForest forest2(temp2, temp_seeds2);

		// フォレストを使って、マッチングを探す
		QMap<RoadVertexDesc, RoadVertexDesc> map1;
		QMap<RoadVertexDesc, RoadVertexDesc> map2;
		findCorrespondence(temp1, &forest1, temp2, &forest2, map1, map2);

		// 非類似度を計算
		float unsimilarity = GraphUtil::computeUnsimilarity(temp1, map1, temp2, map2, 1.0f, 1.0f, 1.0f, 1.0f);

		// 非類似度が最小なら、ベストマッチングとして更新
		if (unsimilarity < min_unsimilarity) {
			min_unsimilarity = unsimilarity;
			min_map1 = map1;
			min_map2 = map2;
			min_roads1 = GraphUtil::copyRoads(temp1);
			min_roads2 = GraphUtil::copyRoads(temp2);

			// シード候補を、正式なシードとして採用
			seeds1 = temp_seeds1;
			seeds2 = temp_seeds2;

			// Importance順に並べたエッジリストから、使用したペアを削除
			for (int j = 0; j < edges1.size(); j++) {
				if (edges1[j] == e1) {
					edges1.removeAt(j);
				}
			}
			for (int j = 0; j < edges2.size(); j++) {
				if (edges2[j] == e2) {
					edges2.removeAt(j);
				}
			}
		}

		// テンポラリの道路網を削除
		delete temp1;
		delete temp2;
	}

	qDebug() << "Min unsimilarity: " << min_unsimilarity;
	qDebug() << "    connectivity: " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 1.0f, 0.0f, 0.0f, 0.0f);
	qDebug() << "    split       : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 1.0f, 0.0f, 0.0f);
	qDebug() << "    angle       : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 0.0f, 1.0f, 0.0f);
	qDebug() << "    distance    : " << GraphUtil::computeUnsimilarity(min_roads1, min_map1, min_roads2, min_map2, 0.0f, 0.0f, 0.0f, 1.0f);

	correspondence = min_map1;
	roads1 = GraphUtil::copyRoads(min_roads1);
	roads2 = GraphUtil::copyRoads(min_roads2);
	delete min_roads1;
	delete min_roads2;

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

	for (int i = 0; i < forest1->getRoots().size(); i++) {
		map1[forest1->getRoots()[i]] = forest2->getRoots()[i];
		map2[forest2->getRoots()[i]] = forest1->getRoots()[i];

		seeds1.push_back(forest1->getRoots()[i]);
		seeds2.push_back(forest2->getRoots()[i]);
	}

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (forest1->getChildren(parent1).size() == 0 && forest2->getChildren(parent2).size() == 0) continue;

		//float theta = findBestAffineTransofrmation(roads1, parent1, tree1, roads2, parent2, tree2);
		float theta = 0.0f;

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
			if (!findBestPairByDirection(theta, roads1, parent1, forest1, map1, roads2, parent2, forest2, map2, false, child1, child2)) break;

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
bool BFSMulti::findBestPairByDirection(float theta, RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2) {
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
		float theta1 = atan2f(dir1.y(), dir1.x()) + theta;
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
 * Imporatanceとsimilarityを使って、エッジのペアを選択する。
 * まず、Top20のImportantエッジを２つの道路網から取得し、それぞれのSimilarityを計算し、
 * 最もSimilarityの高い組み合わせを返却する。
 */
void BFSMulti::getSimilarEdges(RoadGraph* roads1, RoadGraph* roads2, QList<RoadEdgeDesc>& edges1, QList<RoadEdgeDesc>& edges2, RoadEdgeDesc& e1, RoadEdgeDesc& e2) {
	float min_diff = std::numeric_limits<float>::max();
	
	// 道路網１のTop20 Importantエッジに対して、似ている対応エッジを道路網２から探す
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < edges2.size(); j++) {
			float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[i], roads2, edges2[j], 1.0f, 1.0f, 1.0f);
			if (diff < min_diff) {
				min_diff = diff;
				e1 = edges1[i];
				e2 = edges2[j];
			}
		}
	}

	// 道路網２のTop20 Importantエッジに対して、似ている対応エッジを道路網１から探す
	for (int i = 0; i < 20; i++) {
		for (int j = 20; j < edges1.size(); j++) {
			float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[j], roads2, edges2[i], 1.0f, 1.0f, 1.0f);
			if (diff < min_diff) {
				min_diff = diff;
				e1 = edges1[j];
				e2 = edges2[i];
			}
		}
	}

}


