#include "BFS.h"
#include "GraphUtil.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

BFS::BFS(const char* filename1, const char* filename2) {
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

	//createRoads1();
	//createRoads2();

	selected = 0;
	tree1 = NULL;
	tree2 = NULL;
}

BFS::~BFS() {
	delete roads1;
	delete roads2;
	delete tree1;
	delete tree2;
	clearSequence();
}

void BFS::draw(QPainter* painter, int offset, float scale) {
	if (roads1 == NULL) return;

	//drawGraph(painter, roads1, QColor(0, 0, 255), offset, scale);
	//drawGraph(painter, roads2, QColor(255, 0, 0), offset, scale);
	//drawRelation(painter, roads1, &correspondence, roads2, offset, scale);

	drawGraph(painter, sequence[selected], QColor(0, 0, 255), offset, scale);
}

void BFS::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
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
		if (!v->valid) continue;

		int x = (v->getPt().x() + offset) * scale ;
		int y = (-v->getPt().y() + offset) * scale;
		painter->fillRect(x - 1, y - 1, 3, 3, col);
	}
}

void BFS::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc>* correspondence, RoadGraph *roads2, int offset, float scale) {
	if (roads1 == NULL || roads2 == NULL) return;

	painter->setPen(QPen(Qt::black, 1, Qt::DotLine, Qt::RoundCap));

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		if (!v1->valid) continue;

		//if (rand() * 10 < 8) continue;

		RoadVertexDesc v2_desc = correspondence->value(*vi);

		RoadVertex* v2 = roads2->graph[v2_desc];
		if (!v2->valid) continue;

		int x1 = (v1->getPt().x() + offset) * scale;
		int y1 = (-v1->getPt().y() + offset) * scale;
		int x2 = (v2->getPt().x() + offset) * scale;
		int y2 = (-v2->getPt().y() + offset) * scale;
		painter->drawLine(x1, y1, x2, y2);
	}
}

RoadGraph* BFS::interpolate(float t) {
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
			RoadEdge* new_e = new RoadEdge(1, 1);
			new_e->addPoint(roads->graph[parent_new]->getPt());
			new_e->addPoint(roads->graph[new_v_desc]->getPt());
			std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(parent_new, new_v_desc, roads->graph);
			roads->graph[new_e_pair.first] = new_e;

			seeds.push_back(child1);
			seeds_new.push_back(new_v_desc);
		}
	}

	return roads;
}

void BFS::buildTree() {
	// 最も短い距離のペアを探し、そのペアをルートとしてBFSを実施
	float min_dist = std::numeric_limits<float>::max();
	RoadVertexDesc min_v1_desc;
	RoadVertexDesc min_v2_desc;

	RoadVertexIter vi, vend;
	int count = 0;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		count++;
		RoadVertexDesc v2_desc = GraphUtil::findNearestNeighbor(roads2, roads1->graph[*vi]->getPt());
		float dist = (roads1->graph[*vi]->getPt() - roads2->graph[v2_desc]->getPt()).length();
		if (dist < min_dist) {
			min_dist = dist;
			min_v1_desc = *vi;
			min_v2_desc = v2_desc;
		}
	}

	min_v1_desc = 2;
	min_v2_desc = 8;

	// 頂点が１つもない場合は、終了
	if (count == 0) return;

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
}

/**
 * ２つの道路網を、木構造を使ってマッチングさせる。
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFS::findCorrespondence(RoadGraph* roads1, BFSTree* tree1, RoadGraph* roads2, BFSTree* tree2) {
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
			//if (!findBestPair(roads1, parent1, tree1, paired1, roads2, parent2, tree2, paired2, child1, child2)) break;
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
bool BFS::findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
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
		RoadEdge* e2 = new RoadEdge(roads1->graph[e1_desc]->lanes, roads1->graph[e1_desc]->type);
		e2->addPoint(roads2->graph[parent2]->getPt());
		e2->addPoint(roads2->graph[v_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> e2_pair = boost::add_edge(parent2, v_desc, roads2->graph);
		roads2->graph[e2_pair.first] = e2;

		tree2->addChild(parent2, v_desc);

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
		RoadEdge* e1 = new RoadEdge(roads2->graph[e2_desc]->lanes, roads2->graph[e2_desc]->type);
		e1->addPoint(roads1->graph[parent1]->getPt());
		e1->addPoint(roads1->graph[v_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> e1_pair = boost::add_edge(parent1, v_desc, roads1->graph);
		roads2->graph[e1_pair.first] = e1;

		tree1->addChild(parent1, v_desc);

		child1 = v_desc;
		child2 = children2[i];

		return true;
	}

	// ペアなし、つまり、全ての子ノードがペアになっている
	return false;
}

/**
 * 子ノードリスト１と子ノードリスト２から、ベストペアを探し出す。
 * まずは、ペアになっていないノードから候補を探す。
 * 既に、一方のリストが全てペアになっている場合は、当該リストからは、ペアとなっているものも含めて、ベストペアを探す。ただし、その場合は、ペアとなったノードをコピーして、必ず１対１ペアとなるようにする。
 */
bool BFS::findBestPairByDirection(RoadGraph* roads1, RoadVertexDesc parent1, BFSTree* tree1, QMap<RoadVertexDesc, bool> paired1, RoadGraph* roads2, RoadVertexDesc parent2, BFSTree* tree2, QMap<RoadVertexDesc, bool> paired2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
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
		RoadEdge* e2 = new RoadEdge(roads1->graph[e1_desc]->lanes, roads1->graph[e1_desc]->type);
		e2->addPoint(roads2->graph[parent2]->getPt());
		e2->addPoint(roads2->graph[v_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> e2_pair = boost::add_edge(parent2, v_desc, roads2->graph);
		roads2->graph[e2_pair.first] = e2;

		tree2->addChild(parent2, v_desc);

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
		RoadEdge* e1 = new RoadEdge(roads2->graph[e2_desc]->lanes, roads2->graph[e2_desc]->type);
		e1->addPoint(roads1->graph[parent1]->getPt());
		e1->addPoint(roads1->graph[v_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> e1_pair = boost::add_edge(parent1, v_desc, roads1->graph);
		roads2->graph[e1_pair.first] = e1;

		tree1->addChild(parent1, v_desc);

		child1 = v_desc;
		child2 = children2[i];

		return true;
	}

	// ペアなし、つまり、全ての子ノードがペアになっている
	return false;
}

void BFS::selectSequence(int selected) {
	this->selected = selected;
}

void BFS::clearSequence() {
	for (int i = 0; i < sequence.size(); i++) {
		sequence[i]->clear();
		delete sequence[i];
	}
	sequence.clear();
}

RoadGraph* BFS::copyRoads(RoadGraph* roads, BFSTree* tree, int num) {
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
			RoadEdge* e2 = new RoadEdge(roads->graph[e1_desc]->lanes, roads->graph[e1_desc]->type);
			e2->addPoint(new_roads->graph[parent2]->getPt());
			e2->addPoint(new_roads->graph[child2_desc]->getPt());

			std::pair<RoadEdgeDesc, bool> e_pair = boost::add_edge(parent2, child2_desc, new_roads->graph);
			new_roads->graph[e_pair.first] = e2;

			seeds1.push_back(child1_desc);
			seeds2.push_back(child2_desc);
		}
	}

	return new_roads;
}

void BFS::createRoads1() {
	roads1 = new RoadGraph();
	RoadVertex* v1 = new RoadVertex(QVector2D(-900, 900));
	RoadVertexDesc v1_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-800, 700));
	RoadVertexDesc v2_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-300, 200));
	RoadVertexDesc v3_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(0, 400));
	RoadVertexDesc v4_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(200, -200));
	RoadVertexDesc v5_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(900, 200));
	RoadVertexDesc v6_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(400, -400));
	RoadVertexDesc v7_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(0, -900));
	RoadVertexDesc v8_desc = boost::add_vertex(roads1->graph);
	roads1->graph[v8_desc] = v8;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(roads1->graph[v1_desc]->getPt());
	e1->addPoint(roads1->graph[v2_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e1_pair = boost::add_edge(v1_desc, v2_desc, roads1->graph);
	roads1->graph[e1_pair.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(roads1->graph[v2_desc]->getPt());
	e2->addPoint(roads1->graph[v3_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e2_pair = boost::add_edge(v2_desc, v3_desc, roads1->graph);
	roads1->graph[e2_pair.first] = e2;

	RoadEdge* e3 = new RoadEdge(1, 1);
	e3->addPoint(roads1->graph[v1_desc]->getPt());
	e3->addPoint(roads1->graph[v4_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e3_pair = boost::add_edge(v1_desc, v4_desc, roads1->graph);
	roads1->graph[e3_pair.first] = e3;

	RoadEdge* e4 = new RoadEdge(1, 1);
	e4->addPoint(roads1->graph[v3_desc]->getPt());
	e4->addPoint(roads1->graph[v4_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e4_pair = boost::add_edge(v3_desc, v4_desc, roads1->graph);
	roads1->graph[e4_pair.first] = e4;

	RoadEdge* e5 = new RoadEdge(1, 1);
	e5->addPoint(roads1->graph[v3_desc]->getPt());
	e5->addPoint(roads1->graph[v5_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e5_pair = boost::add_edge(v3_desc, v5_desc, roads1->graph);
	roads1->graph[e5_pair.first] = e5;

	RoadEdge* e6 = new RoadEdge(1, 1);
	e6->addPoint(roads1->graph[v5_desc]->getPt());
	e6->addPoint(roads1->graph[v6_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e6_pair = boost::add_edge(v5_desc, v6_desc, roads1->graph);
	roads1->graph[e6_pair.first] = e6;

	RoadEdge* e7 = new RoadEdge(1, 1);
	e7->addPoint(roads1->graph[v5_desc]->getPt());
	e7->addPoint(roads1->graph[v7_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e7_pair = boost::add_edge(v5_desc, v7_desc, roads1->graph);
	roads1->graph[e7_pair.first] = e7;

	RoadEdge* e8 = new RoadEdge(1, 1);
	e8->addPoint(roads1->graph[v7_desc]->getPt());
	e8->addPoint(roads1->graph[v8_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e8_pair = boost::add_edge(v7_desc, v8_desc, roads1->graph);
	roads1->graph[e8_pair.first] = e8;

}

void BFS::createRoads2() {
	roads2 = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(-700, 900));
	RoadVertexDesc v1_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(-600, 600));
	RoadVertexDesc v2_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(-900, 200));
	RoadVertexDesc v3_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(-700, 100));
	RoadVertexDesc v4_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(-800, -200));
	RoadVertexDesc v5_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v5_desc] = v5;

	RoadVertex* v6 = new RoadVertex(QVector2D(-600, -200));
	RoadVertexDesc v6_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v6_desc] = v6;

	RoadVertex* v7 = new RoadVertex(QVector2D(-800, -700));
	RoadVertexDesc v7_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v7_desc] = v7;

	RoadVertex* v8 = new RoadVertex(QVector2D(-500, -800));
	RoadVertexDesc v8_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v8_desc] = v8;

	RoadVertex* v9 = new RoadVertex(QVector2D(0, 500));
	RoadVertexDesc v9_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v9_desc] = v9;

	RoadVertex* v10 = new RoadVertex(QVector2D(100, 600));
	RoadVertexDesc v10_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v10_desc] = v10;

	RoadVertex* v11 = new RoadVertex(QVector2D(600, 0));
	RoadVertexDesc v11_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v11_desc] = v11;

	RoadVertex* v12 = new RoadVertex(QVector2D(900, 300));
	RoadVertexDesc v12_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v12_desc] = v12;

	RoadVertex* v13 = new RoadVertex(QVector2D(800, -100));
	RoadVertexDesc v13_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v13_desc] = v13;

	RoadVertex* v14 = new RoadVertex(QVector2D(400, -600));
	RoadVertexDesc v14_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v14_desc] = v14;

	RoadVertex* v15 = new RoadVertex(QVector2D(500, -900));
	RoadVertexDesc v15_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v15_desc] = v15;

	RoadVertex* v16 = new RoadVertex(QVector2D(800, -400));
	RoadVertexDesc v16_desc = boost::add_vertex(roads2->graph);
	roads2->graph[v16_desc] = v16;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(roads2->graph[v1_desc]->getPt());
	e1->addPoint(roads2->graph[v2_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e1_pair = boost::add_edge(v1_desc, v2_desc, roads2->graph);
	roads2->graph[e1_pair.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(roads2->graph[v2_desc]->getPt());
	e2->addPoint(roads2->graph[v4_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e2_pair = boost::add_edge(v2_desc, v4_desc, roads2->graph);
	roads2->graph[e2_pair.first] = e2;

	RoadEdge* e3 = new RoadEdge(1, 1);
	e3->addPoint(roads2->graph[v3_desc]->getPt());
	e3->addPoint(roads2->graph[v4_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e3_pair = boost::add_edge(v3_desc, v4_desc, roads2->graph);
	roads2->graph[e3_pair.first] = e3;

	RoadEdge* e4 = new RoadEdge(1, 1);
	e4->addPoint(roads2->graph[v4_desc]->getPt());
	e4->addPoint(roads2->graph[v5_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e4_pair = boost::add_edge(v4_desc, v5_desc, roads2->graph);
	roads2->graph[e4_pair.first] = e4;

	RoadEdge* e5 = new RoadEdge(1, 1);
	e5->addPoint(roads2->graph[v4_desc]->getPt());
	e5->addPoint(roads2->graph[v6_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e5_pair = boost::add_edge(v4_desc, v6_desc, roads2->graph);
	roads2->graph[e5_pair.first] = e5;

	RoadEdge* e6 = new RoadEdge(1, 1);
	e6->addPoint(roads2->graph[v5_desc]->getPt());
	e6->addPoint(roads2->graph[v7_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e6_pair = boost::add_edge(v5_desc, v7_desc, roads2->graph);
	roads2->graph[e6_pair.first] = e6;

	RoadEdge* e7 = new RoadEdge(1, 1);
	e7->addPoint(roads2->graph[v6_desc]->getPt());
	e7->addPoint(roads2->graph[v7_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e7_pair = boost::add_edge(v6_desc, v7_desc, roads2->graph);
	roads2->graph[e7_pair.first] = e7;

	RoadEdge* e8 = new RoadEdge(1, 1);
	e8->addPoint(roads2->graph[v6_desc]->getPt());
	e8->addPoint(roads2->graph[v8_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e8_pair = boost::add_edge(v6_desc, v8_desc, roads2->graph);
	roads2->graph[e8_pair.first] = e8;

	RoadEdge* e9 = new RoadEdge(1, 1);
	e9->addPoint(roads2->graph[v2_desc]->getPt());
	e9->addPoint(roads2->graph[v9_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e9_pair = boost::add_edge(v2_desc, v9_desc, roads2->graph);
	roads2->graph[e9_pair.first] = e9;

	RoadEdge* e10 = new RoadEdge(1, 1);
	e10->addPoint(roads2->graph[v9_desc]->getPt());
	e10->addPoint(roads2->graph[v10_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e10_pair = boost::add_edge(v9_desc, v10_desc, roads2->graph);
	roads2->graph[e10_pair.first] = e10;

	RoadEdge* e11 = new RoadEdge(1, 1);
	e11->addPoint(roads2->graph[v9_desc]->getPt());
	e11->addPoint(roads2->graph[v11_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e11_pair = boost::add_edge(v9_desc, v11_desc, roads2->graph);
	roads2->graph[e11_pair.first] = e11;

	RoadEdge* e12 = new RoadEdge(1, 1);
	e12->addPoint(roads2->graph[v11_desc]->getPt());
	e12->addPoint(roads2->graph[v12_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e12_pair = boost::add_edge(v11_desc, v12_desc, roads2->graph);
	roads2->graph[e12_pair.first] = e12;

	RoadEdge* e13 = new RoadEdge(1, 1);
	e13->addPoint(roads2->graph[v11_desc]->getPt());
	e13->addPoint(roads2->graph[v13_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e13_pair = boost::add_edge(v11_desc, v13_desc, roads2->graph);
	roads2->graph[e13_pair.first] = e13;

	RoadEdge* e14 = new RoadEdge(1, 1);
	e14->addPoint(roads2->graph[v11_desc]->getPt());
	e14->addPoint(roads2->graph[v14_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e14_pair = boost::add_edge(v11_desc, v14_desc, roads2->graph);
	roads2->graph[e14_pair.first] = e14;

	RoadEdge* e15 = new RoadEdge(1, 1);
	e15->addPoint(roads2->graph[v14_desc]->getPt());
	e15->addPoint(roads2->graph[v15_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e15_pair = boost::add_edge(v14_desc, v15_desc, roads2->graph);
	roads2->graph[e15_pair.first] = e15;

	RoadEdge* e16 = new RoadEdge(1, 1);
	e16->addPoint(roads2->graph[v13_desc]->getPt());
	e16->addPoint(roads2->graph[v16_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e16_pair = boost::add_edge(v13_desc, v16_desc, roads2->graph);
	roads2->graph[e16_pair.first] = e16;

	RoadEdge* e17 = new RoadEdge(1, 1);
	e17->addPoint(roads2->graph[v15_desc]->getPt());
	e17->addPoint(roads2->graph[v16_desc]->getPt());
	std::pair<RoadEdgeDesc, bool> e17_pair = boost::add_edge(v15_desc, v16_desc, roads2->graph);
	roads2->graph[e17_pair.first] = e17;



}