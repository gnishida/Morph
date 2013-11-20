#include "BFSMulti.h"
#include "GraphUtil.h"
#include "Util.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

BFSMulti::BFSMulti(const char* filename1, const char* filename2) : AbstractBFS(filename1, filename2) {
	forest1 = NULL;
	forest2 = NULL;
}

BFSMulti::~BFSMulti() {
	delete forest1;
	delete forest2;
}

RoadGraph* BFSMulti::interpolate(float t) {
	if (t == 1.0f) return GraphUtil::copyRoads(roads1);
	if (t == 0.0f) return GraphUtil::copyRoads(roads2);

	RoadGraph* roads = new RoadGraph();

	QMap<RoadVertexDesc, QMap<RoadVertexDesc, RoadVertexDesc> > conv;

	std::list<RoadVertexDesc> seeds;
	std::list<RoadVertexDesc> seeds_new;

	for (int i = 0; i < forest1->getRoots().size(); i++) {
		// ルートの頂点を作成
		RoadVertex* v_root = new RoadVertex(roads1->graph[forest1->getRoots()[i]]->getPt() * t + roads2->graph[forest2->getRoots()[i]]->getPt() * (1 - t));
		RoadVertexDesc v_root_desc = boost::add_vertex(roads->graph);
		roads->graph[v_root_desc] = v_root;

		// ルート頂点をシードに入れる
		seeds.push_back(forest1->getRoots()[i]);
		seeds_new.push_back(v_root_desc);
	}

	// 木構造を使って、頂点を登録していく
	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		RoadVertexDesc parent_new = seeds_new.front();
		seeds_new.pop_front();

		// 子ノードリストを取得
		for (int i = 0; i < forest1->getChildren(parent).size(); i++) {
			// 子ノードを取得
			RoadVertexDesc child1 = forest1->getChildren(parent)[i];
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

void BFSMulti::buildTree() {
	float min_dist = std::numeric_limits<float>::max();
	std::vector<RoadVertexDesc> roots1;
	std::vector<RoadVertexDesc> roots2;

	// テンポラリで、手動でルートを指定
	//min_v1_desc = 33;
	//min_v2_desc = 74;
	roots1.push_back(641);
	//roots1.push_back(225);
	roots2.push_back(530);

	if (forest1 != NULL) delete forest1;
	if (forest2 != NULL) delete forest2;
	forest1 = new BFSForest(roads1, roots1);
	forest2 = new BFSForest(roads2, roots2);

	correspondence = findCorrespondence(roads1, forest1, roads2, forest2);

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
QMap<RoadVertexDesc, RoadVertexDesc> BFSMulti::findCorrespondence(RoadGraph* roads1, BFSForest* forest1, RoadGraph* roads2, BFSForest* forest2) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::list<RoadVertexDesc> seeds1;
	std::list<RoadVertexDesc> seeds2;

	for (int i = 0; i < forest1->getRoots().size(); i++) {
		correspondence[forest1->getRoots()[i]] = forest2->getRoots()[i];

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

		QMap<RoadVertexDesc, bool> paired1;
		QMap<RoadVertexDesc, bool> paired2;

		//float theta = findBestAffineTransofrmation(roads1, parent1, tree1, roads2, parent2, tree2);
		float theta = 0.0f;

		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPairByDirection(theta, roads1, parent1, forest1, paired1, roads2, parent2, forest2, paired2, false, child1, child2)) break;

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
bool BFSMulti::findBestPairByDirection(float theta, RoadGraph* roads1, RoadVertexDesc parent1, BFSForest* forest1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSForest* forest2, QMap<RoadVertexDesc, bool> paired2, bool onlyUnpairedNode, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	// 子リストを取得
	std::vector<RoadVertexDesc> children1 = forest1->getChildren(parent1);
	std::vector<RoadVertexDesc> children2 = forest2->getChildren(parent2);

	// エッジの角度が最もちかいペアをマッチさせる
	for (int i = 0; i < children1.size(); i++) {
		if (paired1.contains(children1[i])) continue;
		if (!roads1->graph[children1[i]]->valid) continue;

		QVector2D dir1 = roads1->graph[children1[i]]->getPt() - roads1->graph[parent1]->getPt();
		float theta1 = atan2f(dir1.y(), dir1.x()) + theta;
		for (int j = 0; j < children2.size(); j++) {
			if (paired2.contains(children2[j])) continue;
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
		if (paired1.contains(children1[i])) continue;
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
		if (paired2.contains(children2[i])) continue;
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


