#include "BFS.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

BFS2::BFS2() : AbstractBFS() {
	tree1 = NULL;
	tree2 = NULL;
}

BFS2::~BFS2() {
	delete tree1;
	delete tree2;
}

RoadGraph* BFS2::interpolate(float t) {
	if (t == 1.0f) return GraphUtil::copyRoads(roads1);
	if (t == 0.0f) return GraphUtil::copyRoads(roads2);

	RoadGraph* roads = new RoadGraph();

	QMap<RoadVertexDesc, QMap<RoadVertexDesc, RoadVertexDesc> > conv;

	// ルートの頂点を作成
	RoadVertex* v_root = new RoadVertex(roads1->graph[tree1->getRoot()]->getPt() * t + roads2->graph[tree2->getRoot()]->getPt() * (1 - t));
	RoadVertexDesc v_root_desc = boost::add_vertex(roads->graph);
	roads->graph[v_root_desc] = v_root;

	// ルート頂点をシードに入れる
	std::list<RoadVertexDesc> seeds;
	std::list<RoadVertexDesc> seeds_new;
	seeds.push_back(tree1->getRoot());
	seeds_new.push_back(v_root_desc);

	// 木構造を使って、頂点を登録していく
	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		RoadVertexDesc parent_new = seeds_new.front();
		seeds_new.pop_front();

		// 子ノードリストを取得
		for (int i = 0; i < tree1->getChildren(parent).size(); i++) {
			// 子ノードを取得
			RoadVertexDesc child1 = tree1->getChildren(parent)[i];
			if (!roads1->graph[child1]->valid) continue;

			// 対応ノードを取得
			RoadVertexDesc child2 = correspondence[child1];

			// 子ノードの頂点を作成
			RoadVertex* new_v = new RoadVertex(roads1->graph[child1]->getPt() * t + roads2->graph[child2]->getPt() * (1 - t));
			RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
			roads->graph[new_v_desc] = new_v;

			// エッジを作成
			GraphUtil::addEdge(roads, parent_new, new_v_desc,1, 1, false);

			seeds.push_back(child1);
			seeds_new.push_back(new_v_desc);
		}
	}

	return roads;
}

void BFS2::init() {
	float min_dist = std::numeric_limits<float>::max();
	RoadVertexDesc min_v1_desc;
	RoadVertexDesc min_v2_desc;

	// テンポラリで、手動でルートを指定
	//min_v1_desc = 33;
	//min_v2_desc = 74;
	min_v1_desc = 15;
	min_v2_desc = 5;

	//findBestRoots(roads1, roads2, min_v1_desc, min_v2_desc);

	if (tree1 != NULL) delete tree1;
	if (tree2 != NULL) delete tree2;
	tree1 = new BFSTree(roads1, min_v1_desc);
	tree2 = new BFSTree(roads2, min_v2_desc);

	correspondence = findCorrespondence(roads1, tree1, roads2, tree2);

	// シーケンスを生成
	clearSequence();
	for (int i = 0; i <= 20; i++) {
		float t = 1.0f - (float)i * 0.05f;

		sequence.push_back(interpolate(t));
	}

	selected = 0;
}

/**
 * ２つの道路網を、木構造を使ってマッチングさせる。
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFS2::findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2) {
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
			if (!findBestPairByDirection(roads1, parent1, tree1, paired1, roads2, parent2, tree2, paired2, false, child1, child2)) break;

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
bool BFS2::findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2) {
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

	if (onlyUnpairedNode) return false;

	// グラフ１の方で、ペアが見つかってないノードのために、無理やりペアを作る
	for (int i = 0; i < children1.size(); i++) {
		if (paired1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		// コピーされたノードで、ペアになっていないものは、この時点で捨てる。
		if (roads1->graph[children1[i]]->virt) {
			roads1->graph[GraphUtil::getEdge(roads1, parent1, children1[i])]->valid = false;
			tree1->removeSubTree(children1[i]);
			continue;
		}

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

		//if (min_angle < std::numeric_limits<float>::max()) {
		if (min_angle < M_PI / 2.0f) {	// 角度の差が９０度を超えた場合は、無理やりマッチングさせない。
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

		// コピーされたノードで、ペアになっていないものは、この時点で捨てる。
		if (roads2->graph[children2[i]]->virt) {
			roads2->graph[GraphUtil::getEdge(roads2, parent2, children2[i])]->valid = false;
			tree2->removeSubTree(children2[i]);
			continue;
		}

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

		//if (min_angle < std::numeric_limits<float>::max()) {
		if (min_angle < M_PI / 2.0f) {	// 角度の差が９０度を超えた場合は、無理やりマッチングさせない。
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

RoadGraph* BFS2::copyRoads(RoadGraph* roads, BFSTree* tree, int num) {
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

void BFS2::findBestRoots(RoadGraph* roads1, RoadGraph* roads2, RoadVertexDesc& root1, RoadVertexDesc& root2) {
	int min_score = std::numeric_limits<int>::max();

	RoadVertexIter vi1, vend1;
	for (boost::tie(vi1, vend1) = boost::vertices(roads1->graph); vi1 != vend1; ++vi1) {
		if (!roads1->graph[*vi1]->valid) continue;

		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(roads2->graph); vi2 != vend2; ++vi2) {
			if (!roads2->graph[*vi2]->valid) continue;

			int score = computeUnbalanceness(roads1, *vi1, roads2, *vi2);
			if (score < min_score) {
				min_score = score;
				root1 = *vi1;
				root2 = *vi2;
			}
		}
	}
}

/**
 * 指定した頂点配下を比べて、アンバランス度を計算する。
 */
int BFS2::computeUnbalanceness(RoadGraph* roads1,  RoadVertexDesc node1, RoadGraph* roads2,  RoadVertexDesc node2) {
	int score = 0;

	// 木構造を作成する
	BFSTree tree1(roads1, node1);
	BFSTree tree2(roads2, node2);

	std::list<RoadVertexDesc> seeds1;
	seeds1.push_back(node1);
	std::list<RoadVertexDesc> seeds2;
	seeds2.push_back(node2);

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		// 子リストを取得
		std::vector<RoadVertexDesc> children1 = tree1.getChildren(parent1);
		std::vector<RoadVertexDesc> children2 = tree2.getChildren(parent2);

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (children1.size() == 0 && children2.size() == 0) continue;

		QMap<RoadVertexDesc, bool> paired1;
		QMap<RoadVertexDesc, bool> paired2;

		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPairByDirection(roads1, parent1, &tree1, paired1, roads2, parent2, &tree2, paired2, true, child1, child2)) break;
			
			paired1[child1] = true;
			paired2[child2] = true;
			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}

		// ペアにならなかったノードについて、ペナルティをカウントする
		for (int i = 0; i < children1.size(); i++) {
			if (paired1.contains(children1[i])) continue;
			if (!roads1->graph[children1[i]]->valid) continue;

			score += tree1.getTreeSize(children1[i]);
		}
		for (int i = 0; i < children2.size(); i++) {
			if (paired2.contains(children2[i])) continue;
			if (!roads2->graph[children2[i]]->valid) continue;

			score += tree2.getTreeSize(children2[i]);
		}
	}

	return score;
}
