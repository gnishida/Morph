#include "BFSProp.h"
#include "GraphUtil.h"
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
	/*
	QMap<RoadVertexDesc, RoadVertexDesc> map1;
	QMap<RoadVertexDesc, RoadVertexDesc> map2;
	GraphUtil::computeMinUnsimilarity(roads1, map1, roads2, map2);

	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = map1.begin(); it != map1.end(); ++it) {
		correspondence[it.key()] = it.value();
	}
	*/

	/*
	// 一旦、invalidのノードなどを削除しておく
	RoadGraph* temp1 = GraphUtil::copyRoads(roads1);
	delete roads1;
	roads1 = temp1;
	RoadGraph* temp2 = GraphUtil::copyRoads(roads2);
	delete roads2;
	roads2 = temp2;

	// roads1を、最もエリアの小さいグリッド型に無理やり変換する
	float min_area = std::numeric_limits<float>::max();
	RoadVertexDesc min_v1;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadGraph* grid1 = GraphUtil::convertToGridNetwork(roads1, *vi);
		BBox box = GraphUtil::getBoundingBox(roads1);
		float area = (box.maxPt.x() - box.minPt.x()) * (box.maxPt.y() - box.minPt.y());
		if (area < min_area) {
			min_area = area;
			min_v1 = *vi;
		}
	}
	temp1 = GraphUtil::convertToGridNetwork(roads1, min_v1);
	delete roads1;
	roads1 = temp1;
	*/

	clearSequence();
	sequence.push_back(GraphUtil::copyRoads(roads1));


	// シーケンスを生成
	/*
	clearSequence();
	for (int i = 0; i <= 20; i++) {
		float t = 1.0f - (float)i * 0.05f;

		sequence.push_back(interpolate(t));
	}
	*/
}

/**
 * ２つの道路網を、エッジペアを使ってマッチングさせる。
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFSProp::findCorrespondence(RoadGraph* roads1, QList<RoadEdgeDesc>& chosen1, RoadGraph* roads2, QList<RoadEdgeDesc>& chosen2) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	correspondence[tree1->getRoot()] = tree2->getRoot();

	std::list<RoadVertexDesc> seeds1;
	seeds1.push_back(tree1->getRoot());
	std::list<RoadVertexDesc> seeds2;
	seeds2.push_back(tree2->getRoot());

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (tree1->getChildren(parent1).size() == 0 && tree2->getChildren(parent2).size() == 0) continue;

		QMap<RoadVertexDesc, bool> paired1;
		QMap<RoadVertexDesc, bool> paired2;

		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPairByDirection(roads1, parent1, tree1, paired1, roads2, parent2, tree2, paired2, child1, child2)) break;

			correspondence[child1] = child2;
			paired1[child1] = true;
			paired2[child2] = true;
			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}
	}

	return correspondence;
}

/**
 * 子ノードリスト１と子ノードリスト２から、ベストペアを探し出す。
 * まずは、ペアになっていないノードから候補を探す。
 * 既に、一方のリストが全てペアになっている場合は、当該リストからは、ペアとなっているものも含めて、ベストペアを探す。ただし、その場合は、ペアとなったノードをコピーして、必ず１対１ペアとなるようにする。
 */
bool BFSProp::findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	// 子リストを取得
	std::vector<RoadVertexDesc> children1 = tree1->getChildren(parent1);
	std::vector<RoadVertexDesc> children2 = tree2->getChildren(parent2);

	// エッジの角度が最もちかいペアをマッチさせる
	for (int i = 0; i < children1.size(); i++) {
		if (paired1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		QVector2D dir1 = roads1->graph[children1[i]]->getPt() - roads1->graph[parent1]->getPt();
		for (int j = 0; j < children2.size(); j++) {
			if (paired2.contains(children2[j])) continue;
			if (!roads2->graph[children2[j]]->valid) continue;

			QVector2D dir2 = roads2->graph[children2[j]]->getPt() - roads2->graph[parent2]->getPt();

			float angle = GraphUtil::diffAngle(dir1, dir2);
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

	// グラフ１の方で、ペアが見つかってないノードのために、無理やりペアを作る
	for (int i = 0; i < children1.size(); i++) {
		if (paired1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		min_angle = std::numeric_limits<float>::max();
		QVector2D dir1 = roads1->graph[children1[i]]->getPt() - roads1->graph[parent1]->getPt();

		// ペア済みのノードも含めて、最も角度の近い相手を探す
		for (int j = 0; j < children2.size(); j++) {
			if (!roads2->graph[children2[j]]->valid) continue;

			QVector2D dir2 = roads2->graph[children2[j]]->getPt() - roads2->graph[parent2]->getPt();

			float angle = GraphUtil::diffAngle(dir1, dir2);
			if (angle < min_angle) {
				min_angle = angle;
				min_id1 = i;
				min_id2 = j;
			}
		}

		if (min_angle < std::numeric_limits<float>::max()) {
			// 子ノード（子孫も含めて）をコピーする
			RoadVertexDesc v_desc = tree2->copySubTree(parent2, children2[min_id2], parent2);

			child1 = children1[min_id1];
			child2 = v_desc;

			return true;
		} else {
			// 相手グラフに子ノードが１つもない場合は、親ノードとマッチさせる
			RoadVertex* v = new RoadVertex(roads2->graph[parent2]->getPt());
			RoadVertexDesc v_desc = boost::add_vertex(roads2->graph);
			roads2->graph[v_desc] = v;

			RoadEdgeDesc e1_desc = GraphUtil::getEdge(roads1, parent1, children1[i]);

			// 相手の親ノードと子ノードの間にエッジを作成する
			GraphUtil::addEdge(roads2, parent2, v_desc, roads1->graph[e1_desc]->lanes, roads1->graph[e1_desc]->type, roads1->graph[e1_desc]->oneWay);

			tree2->addChild(parent2, v_desc);

			child1 = children1[i];
			child2 = v_desc;

			return true;
		}
	}

	// グラフ２の方で、ペアが見つかってないノードのために、無理やりペアを作る
	for (int i = 0; i < children2.size(); i++) {
		if (paired2.contains(children2[i])) continue;
		if (!roads2->graph[children2[i]]->valid) continue;

		min_angle = std::numeric_limits<float>::max();
		QVector2D dir2 = roads2->graph[children2[i]]->getPt() - roads2->graph[parent2]->getPt();

		// ペア済みのノードも含めて、最も角度の近い相手を探す
		for (int j = 0; j < children1.size(); j++) {
			if (!roads1->graph[children1[j]]->valid) continue;

			QVector2D dir1 = roads1->graph[children1[j]]->getPt() - roads1->graph[parent1]->getPt();

			float angle = GraphUtil::diffAngle(dir1, dir2);
			if (angle < min_angle) {
				min_angle = angle;
				min_id2 = i;
				min_id1 = j;
			}
		}

		if (min_angle < std::numeric_limits<float>::max()) {
			// 子ノード（子孫も含めて）をコピーする
			RoadVertexDesc v_desc = tree1->copySubTree(parent1, children1[min_id1], parent1);

			child1 = v_desc;
			child2 = children2[min_id2];

			return true;
		} else {
			// 相手グラフに子ノードが１つもない場合は、親ノードとマッチさせる
			RoadVertex* v = new RoadVertex(roads1->graph[parent1]->getPt());
			RoadVertexDesc v_desc = boost::add_vertex(roads1->graph);
			roads1->graph[v_desc] = v;

			RoadEdgeDesc e2_desc = GraphUtil::getEdge(roads2, parent2, children2[i]);

			// 相手の親ノードと子ノードの間にエッジを作成する
			GraphUtil::addEdge(roads1, parent1, v_desc, roads2->graph[e2_desc]->lanes, roads2->graph[e2_desc]->type, roads2->graph[e2_desc]->oneWay);

			tree1->addChild(parent1, v_desc);

			child1 = v_desc;
			child2 = children2[i];

			return true;
		}
	}

	// ペアなし、つまり、全ての子ノードがペアになっている
	return false;
}

RoadGraph* BFSProp::copyRoads(RoadGraph* roads, BFSTree* tree, int num) {
	RoadGraph* new_roads = new RoadGraph();

	std::list<RoadVertexDesc> seeds1;
	seeds1.push_back(tree->getRoot());

	std::list<RoadVertexDesc> seeds2;
	RoadVertex* v_root = new RoadVertex(roads->graph[tree->getRoot()]->getPt());
	RoadVertexDesc v_root_desc = boost::add_vertex(new_roads->graph);
	new_roads->graph[v_root_desc] = v_root;
	seeds2.push_back(v_root_desc);


	int count = 0;
	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();

		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		count++;
		if (count > num) break;

		for (int i = 0; i < tree->getChildren(parent1).size(); i++) {
			RoadVertexDesc child1_desc = tree->getChildren(parent1)[i];

			RoadVertex* child2 = new RoadVertex(roads->graph[child1_desc]->getPt());
			RoadVertexDesc child2_desc = boost::add_vertex(new_roads->graph);
			new_roads->graph[child2_desc] = child2;

			RoadEdgeDesc e1_desc = GraphUtil::getEdge(roads, parent1, child1_desc);

			GraphUtil::addEdge(new_roads, parent2, child2_desc, roads->graph[e1_desc]->lanes, roads->graph[e1_desc]->type, roads->graph[e1_desc]->oneWay);

			seeds1.push_back(child1_desc);
			seeds2.push_back(child2_desc);
		}
	}

	return new_roads;
}
