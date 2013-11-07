#include "MMT.h"
#include "GraphUtil.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

VertexPriority::VertexPriority() {
	desc = -1;
	priority = 0;
}

VertexPriority::VertexPriority(RoadVertexDesc desc, float priority) {
	this->desc = desc;
	this->priority = priority;
}

MMT::MMT(Morph* morph, const char* filename1, const char* filename2) {
	this->morph = morph;

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
}

void MMT::draw(QPainter* painter, int offset, float scale) {
	if (roads1 == NULL) return;

	drawGraph(painter, roads1, QColor(0, 0, 255), offset, scale);
}

void MMT::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
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

void MMT::buildTree() {
	// 頂点の中で、degreeが1のものをcollapseしていく
	collapse(roads1);

	// 生き残っている頂点を探す。
	std::list<RoadVertexDesc> v_list;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		v_list.push_back(*vi);
	}

	expand(roads1);
}

void MMT::buildTree2() {
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

	// 木構造を構築する
	tree1 = bfs(roads1, min_v1_desc);
	tree2 = bfs(roads2, min_v2_desc);

	correspondence = findCorrespondence(roads1, min_v1_desc, &tree1, roads2, min_v2_desc, &tree2);
}

/**
 * BFSアプローチで、ルートからノードをたどり、木構造を作成する
 */
QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > MMT::bfs(RoadGraph* roads, RoadVertexDesc root) {
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > tree;

	std::list<RoadVertexDesc> seeds;
	seeds.push_back(root);
	QMap<RoadVertexDesc, bool> visited;

	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		std::vector<RoadVertexDesc> children;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(parent, roads->graph); ei != eend; ++ei) {
			RoadVertexDesc child = boost::target(*ei, roads->graph);
			if (visited.contains(child)) {
				// 対象ノードが訪問済みの場合、対象ノードをコピーして子ノードにする
				RoadVertex* v = new RoadVertex(roads->graph[child]->getPt());
				RoadVertexDesc child2 = boost::add_vertex(roads->graph);
				roads->graph[child2] = v;

				children.push_back(child2);
			} else {
				visited[child] = true;

				children.push_back(child);

				seeds.push_back(child);
			}
		}

		tree[parent] = children;
	}

	return tree;
}

/**
 * ２つの道路網を、木構造を使ってマッチングさせる。
 */
QMap<RoadVertexDesc, RoadVertexDesc> MMT::findCorrespondence(RoadGraph* roads1, RoadVertexDesc root1, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree1, RoadGraph* roads2, RoadVertexDesc root2, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* tree2) {
	QMap<RoadVertexDesc, RoadVertexDesc> correspondence;

	std::list<RoadVertexDesc> seeds1;
	seeds1.push_back(root1);
	std::list<RoadVertexDesc> seeds2;
	seeds2.push_back(root2);

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		// 子ノードのリストを取得
		std::vector<RoadVertexDesc> children1 = tree1->value(parent1);
		std::vector<RoadVertexDesc> children2 = tree2->value(parent2);

		// どちらのノードにも、子ノードがない場合は、スキップ
		if (children1.size() == 0 && children2.size() == 0) continue;

		// ノード１に子ノードがない場合は、ノード１をコピーして子ノードとする
		if (children1.size() == 0) {
			RoadVertex* c = new RoadVertex(roads1->graph[parent1]->getPt());
			RoadVertexDesc child1 = boost::add_vertex(roads1->graph);
			roads1->graph[child1] = c;
			children1.push_back(child1);
		}

		// ノード２に子ノードがない場合は、ノード１をコピーして子ノードとする
		if (children2.size() == 0) {
			RoadVertex* c = new RoadVertex(roads2->graph[parent1]->getPt());
			RoadVertexDesc child2 = boost::add_vertex(roads2->graph);
			roads1->graph[child2] = c;
			children2.push_back(child2);
		}

		// ペアになったかのフラグ
		std::vector<bool> paired1;
		std::vector<bool> paired2;
		for (int i = 0; i < children1.size(); i++) {
			paired1.push_back(false);
		}
		for (int i = 0; i < children2.size(); i++) {
			paired2.push_back(false);
		}

		while (true) {
			RoadVertexDesc child1, child2;
			if (!findBestPair(roads1, parent1, &children1, &paired1, roads2, parent2, &children2, &paired2, child1, child2)) break;

			correspondence[child1] = child2;

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
bool MMT::findBestPair(RoadGraph* roads1, RoadVertexDesc parent1, std::vector<RoadVertexDesc>* children1, std::vector<bool>* paired1, RoadGraph* roads2, RoadVertexDesc parent2, std::vector<RoadVertexDesc>* children2, std::vector<bool>* paired2, RoadVertexDesc& child1, RoadVertexDesc& child2) {
	float min_angle = std::numeric_limits<float>::max();
	int min_id1;
	int min_id2;

	for (int i = 0; i < children1->size(); i++) {
		if (paired1->at(i)) continue;

		QVector2D dir1 = roads1->graph[children1->at(i)]->getPt() - roads1->graph[parent1]->getPt();
		for (int j = 0; j < children2->size(); j++) {
			if (paired2->at(j)) continue;

			QVector2D dir2 = roads2->graph[children2->at(j)]->getPt() - roads2->graph[parent2]->getPt();

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

		child1 = children1->at(min_id1);
		child2 = children2->at(min_id2);

		return true;
	}

	// ベストペアが見つからない、つまり、一方のリストが、全てペアになっている場合
	for (int i = 0; i < children1->size(); i++) {
		QVector2D dir1 = roads1->graph[children1->at(i)]->getPt() - roads1->graph[parent1]->getPt();

		for (int j = 0; j < children2->size(); j++) {
			// 両方ともペアになっている場合は、スキップ
			if (paired1->at(i) && paired2->at(j)) continue;

			QVector2D dir2 = roads2->graph[children2->at(j)]->getPt() - roads2->graph[parent2]->getPt();

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
		if (paired1->at(min_id1)) {
			// 子ノードをコピー
			RoadVertex* v = new RoadVertex(roads1->graph[children1->at(min_id1)]->getPt());
			child1 = boost::add_vertex(roads1->graph);
			roads1->graph[child1] = v;

			children1->push_back(child1);
			paired1->push_back(true);
		} else {
			child1 = children1->at(min_id1);
			paired1->at(min_id1) = true;
		}

		if (paired2->at(min_id2)) {
			// 子ノードをコピー
			RoadVertex* v = new RoadVertex(roads2->graph[children2->at(min_id2)]->getPt());
			child2 = boost::add_vertex(roads2->graph);
			roads2->graph[child2] = v;

			children2->push_back(child2);
			paired2->push_back(true);
		} else {
			child2 = children2->at(min_id2);
			paired2->at(min_id2) = true;
		}

		child1 = children1->at(min_id1);
		child2 = children2->at(min_id2);

		return true;
	}

	// ペアなし、つまり、全ての子ノードがペアになっている
	return false;
}

/**
 * 頂点を、順番にcollapseしていく。
 * ただし、当該頂点から出るエッジの長さが短いものから、優先的にcollapseしていく。
 */
void MMT::collapse(RoadGraph* roads) {
	qDebug() << "collapse start.";

	RoadOutEdgeIter oei, oeend;
	for (boost::tie(oei, oeend) = boost::out_edges(33, roads->graph); oei != oeend; ++oei) {
		RoadVertexDesc tgt = boost::target(*oei, roads->graph);
		int k = 0;
	}

	int count = 0;

	while (true) {
		if (count == 27) {
			int k = 0;
		}

		float min_len = std::numeric_limits<float>::max();
		RoadEdgeDesc min_e_desc;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			float len = roads->graph[*ei]->getLength();
			
			if (len < min_len) {
				min_len = len;
				min_e_desc = *ei;
			}
		}

		if (min_len == std::numeric_limits<float>::max()) break;

		GraphUtil::collapseEdge(roads, min_e_desc);

		// 再描画
		morph->update();

		qDebug() << "remove edge." << (++count);

		// 300ミリ秒待機
		QTest::qWait(300);
	}

	qDebug() << "collapse done.";
}

/**
 * Collapseした道路網を、親子関係の木構造を使って、元に戻す。
 */
void MMT::expand(RoadGraph* roads) {
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

		// 再描画
		morph->update();

		// 300ミリ秒待機
		QTest::qWait(300);
	}

	qDebug() << "expand done.";
}