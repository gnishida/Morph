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

	/*
	drawGraph(painter, roads1, QColor(0, 0, 255), offset, scale);
	drawGraph(painter, roads2, QColor(255, 0, 0), offset, scale);
	drawRelation(painter, roads1, &correspondence, roads2, offset, scale);
	*/
	
	drawGraph(painter, sequence[selected], QColor(0, 0, 255), offset, scale);
	/*
	RoadGraph* interpolated = interpolate(t);
	drawGraph(painter, interpolated, QColor(0, 0, 255), offset, scale);
	if (t > 0.0f && t < 1.0f) {
		interpolated->clear();
		delete interpolated;
	}
	*/
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

		int x = (v->getPt().x() + offset) * scale;
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

		if (rand() * 10 < 8) continue;

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

		// 子ノードのリストを取得
		std::vector<RoadVertexDesc> children1 = tree1->getChildren(parent1);
		std::vector<RoadVertexDesc> children2 = tree2->getChildren(parent2);

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (children1.size() == 0 && children2.size() == 0) continue;

		// 子ノード同士でペアを探す
		QMap<RoadVertexDesc, RoadVertexDesc> corr;
		if (children1.size() < children2.size()) {
			corr = findPairs(roads1, tree1, parent1, children1, roads2, tree2, parent2, children2);
		} else {
			corr = findPairs(roads2, tree2, parent2, children2, roads1, tree1, parent1, children1);
		}

		for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = corr.begin(); it != corr.end(); ++it) {
			RoadVertexDesc child1 = it.key();
			RoadVertexDesc child2 = it.value();

			correspondence[child1] = child2;
			seeds1.push_back(child1);
			seeds2.push_back(child2);
		}		
	}

	return correspondence;
}

/**
 * 子ノードリスト同士で、マッチングさせる
 * children1の方が、子供の数が少ないという前提
 */
QMap<RoadVertexDesc, RoadVertexDesc> BFS::findPairs(RoadGraph* roads, BFSTree* tree1, RoadVertexDesc parent1, std::vector<RoadVertexDesc> children1, RoadGraph* roads2, BFSTree* tree2, RoadVertexDesc parent2, std::vector<RoadVertexDesc> children2) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	// ペアになったかのフラグ
	std::vector<bool> paired1;
	std::vector<bool> paired2;
	for (int i = 0; i < children1.size(); i++) {
		paired1.push_back(false);
	}
	for (int i = 0; i < children2.size(); i++) {
		paired2.push_back(false);
	}

	// 子ノードリスト１に対して、それぞれペアを探す
	for (int i = 0; i < children1.size(); i++) {
		int height1 = tree1->getHeight(children1[i]);
		int min_diff = std::numeric_limits<int>::max();
		int min_id;
		RoadVertexDesc child2;

		for (int j = 0; j < children2.size(); j++) {
			if (paired2[j]) continue;

			int diff = abs(tree2->getHeight(children2[j]) - height1);
			if (diff < min_diff) {
				min_diff = diff;
				min_id = j;
			}
		}

		paired2[min_id] = true;
		correspondence[children1[i]] = children2[min_id];
	}

	// 子ノードリスト２に対して、ペアになっていないノードがあれば、ペアを探す
	for (int i = 0; i < children2.size(); i++) {
		if (paired2[i]) continue;

		int height2 = tree2->getHeight(children2[i]);
		int min_diff = std::numeric_limits<int>::max();
		int min_id;
		RoadVertexDesc child1;

		for (int j = 0; j < children1.size(); j++) {
			int diff = abs(tree1->getHeight(children1[j]) - height2);
			if (diff < min_diff) {
				min_diff = diff;
				min_id = j;
			}
		}
				
		// children1[index]のノードをコピーして、マッチさせる
		RoadVertexDesc v1_desc = tree1->copySubTree(children1[min_id], parent1);
		correspondence[v1_desc] = children2[i];
	}
	
	return correspondence;
}

/**
 * 子ノードリスト１と子ノードリスト２から、ベストペアを探し出す。
 * まずは、ペアになっていないノードから候補を探す。
 * 既に、一方のリストが全てペアになっている場合は、当該リストからは、ペアとなっているものも含めて、ベストペアを探す。ただし、その場合は、ペアとなったノードをコピーして、必ず１対１ペアとなるようにする。
 */
bool BFS::findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, std::vector<bool>* paired1, RoadGraph* roads2, RoadVertexDesc parent2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2, std::vector<bool>* paired2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	// 子リストを取得
	std::vector<RoadVertexDesc> children1 = tree1->value(parent1);
	std::vector<RoadVertexDesc> children2 = tree2->value(parent2);

	for (int i = 0; i < children1.size(); i++) {
		if (paired1->at(i)) continue;

		QVector2D dir1 = roads1->graph[children1[i]]->getPt() - roads1->graph[parent1]->getPt();
		for (int j = 0; j < children2.size(); j++) {
			if (paired2->at(j)) continue;

			QVector2D dir2 = roads2->graph[children2[j]]->getPt() - roads2->graph[parent2]->getPt();

			float angle = fabs(atan2(dir1.y(), dir1.x()) - atan2(dir2.y(), dir2.x()));
			if (angle < min_angle) {
				min_angle = angle;
				min_id1 = i;
				min_id2 = j;
			}
		}
	}
	
	// ベストペアが見つかったか、チェック
	if (min_angle < std::numeric_limits<float>::max()) {
		paired1->at(min_id1) = true;
		paired2->at(min_id2) = true;

		child1 = children1[min_id1];
		child2 = children2[min_id2];

		return true;
	}

	// ベストペアが見つからない、つまり、一方のリストが、全てペアになっている場合
	for (int i = 0; i < children1.size(); i++) {
		if (paired1->at(i)) continue;

		// 相手の親ノードをコピーしてマッチさせる
		RoadVertex* v = new RoadVertex(roads2->graph[parent2]->getPt());
		RoadVertexDesc v_desc = boost::add_vertex(roads2->graph);
		roads2->graph[v_desc] = v;

		children2.push_back(v_desc);

		paired1->at(i) = true;
		paired2->push_back(true);

		// 子ノードリストが更新されたので、木構造も更新する
		tree2->insert(parent2, children2);
		tree2->insert(v_desc, std::vector<RoadVertexDesc>());

		child1 = children1[i];
		child2 = v_desc;

		return true;
	}

	for (int i = 0; i < children2.size(); i++) {
		if (paired2->at(i)) continue;

		// 相手の親ノードをコピーしてマッチさせる
		RoadVertex* v = new RoadVertex(roads1->graph[parent1]->getPt());
		RoadVertexDesc v_desc = boost::add_vertex(roads1->graph);
		roads1->graph[v_desc] = v;

		children1.push_back(v_desc);

		paired2->at(i) = true;
		paired1->push_back(true);

		// 子ノードリストが更新されたので、木構造も更新する
		tree1->insert(parent1, children1);
		tree1->insert(v_desc, std::vector<RoadVertexDesc>());

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