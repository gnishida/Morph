#include "Morph.h"
#include <qset.h>
#include <qpainter.h>
#include <qcolor.h>
#include <limits>

Morph::Morph(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	timer = new QTimer(this);

	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	interpolated_roads = NULL;
	roadsA = NULL;
	roadsB = NULL;
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	/*
	drawGraph(&painter, roads1, QColor(0, 0, 255));
	drawGraph(&painter, roads2, QColor(255, 0, 0));
	drawRelation(&painter, roads1, neighbor1, roads2, neighbor2);
	*/

	//drawGraph(&painter, interpolated_roads, QColor(0, 0, 255), 5300, 0.05f);
	drawGraph(&painter, roadsB, QColor(0, 0, 255), 5300, 0.05f);
}

void Morph::drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale) {
	if (roads == NULL) return;

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(QPen(col, 2, Qt::SolidLine, Qt::RoundCap));
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

		int x = (v->getPt().x() + offset) / 4;
		int y = (-v->getPt().y() + offset) / 4;
		painter->fillRect(x - 3, y - 3, 6, 6, col);
	}
}

void Morph::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2) {
	if (roads1 == NULL || roads2 == NULL) return;

	painter->setPen(QPen(Qt::black, 2, Qt::DotLine, Qt::RoundCap));

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		RoadVertex* v2 = roads2->graph[neighbor1[*vi]];

		painter->drawLine(v1->getPt().x(), v1->getPt().y(), v2->getPt().x(), v2->getPt().y());
	}
}

void Morph::start() {
	FILE* fp1 = fopen("roads1.gsm", "rb");
	FILE* fp2 = fopen("roads2.gsm", "rb");
	if (roadsA != NULL) {
		delete roadsA;
	}
	if (roadsB != NULL) {
		delete roadsB;
	}
	//roadsA = buildGraph1();
	//roadsB = buildGraph2();
	roadsA = new RoadGraph();
	roadsA->load(fp1, 2);
	roadsB = new RoadGraph();
	roadsB->load(fp2, 2);

	// 第１ステップ：各頂点について、直近の対応を探す
	neighbor1 = findNearestNeighbors(roadsA, roadsB);
	neighbor2 = findNearestNeighbors(roadsB, roadsA);

	// ここで、直近が遠すぎる場合は、近くに頂点を強制的に追加する。

	// 第２ステップ：相思相愛の頂点をマークする
	checkExclusivePair(roadsA, &neighbor1, &neighbor2);
	checkExclusivePair(roadsB, &neighbor2, &neighbor1);

	// 第３ステップ：一人ぼっちの頂点について、可能なら相思相愛にする
	changeAloneToPair(roadsA, &neighbor1, roadsB, &neighbor2);
	changeAloneToPair(roadsB, &neighbor2, roadsA, &neighbor1);

	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = neighbor1.begin(); it != neighbor1.end(); ++it) {
		int aaa = it.key();
		int kkk = neighbor1[it.key()];
		int ccc = 0;
	}
	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = neighbor2.begin(); it != neighbor2.end(); ++it) {
		int aaa = it.key();
		int kkk = neighbor2[it.key()];
		int ccc = 0;
	}

	// 第４ステップ：重複頂点に対し、頂点を増やして１対１対応にさせる
	augmentGraph(roadsA, &neighbor1, roadsB, &neighbor2);
	augmentGraph(roadsB, &neighbor2, roadsA, &neighbor1);

	// 兄弟がない頂点についても、擬似的に、自分自身１人を兄弟として登録する
	setupSiblings(roadsA);
	setupSiblings(roadsB);

	// 第５ステップ：エッジの対応チェック
	updateEdges(roadsA, &neighbor1, roadsB, &neighbor2);
	updateEdges(roadsB, &neighbor2, roadsA, &neighbor1);

	// 第６ステップ：エッジの対応チェック２
	updateEdges2(roadsA, &neighbor1, roadsB, &neighbor2);
	updateEdges2(roadsB, &neighbor2, roadsA, &neighbor1);

	t = 1.0f;

	timer->start(100);
}

void Morph::tick() {
	if (interpolated_roads != NULL) {
		delete interpolated_roads;
	}
	interpolated_roads = interpolate(roadsA, &neighbor1, roadsB, &neighbor2, t);

	update();

	t -= 0.02f;
	if (t < 0.0f) {
		t = 0.0f;
		timer->stop();
	}
}

RoadGraph* Morph::interpolate(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, float t) {
	RoadGraph* roads = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// roads1を元に、頂点を登録していく
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		RoadVertex* v2 = roads2->graph[neighbor1->value(*vi)];

		// 頂点を登録する
		RoadVertex* new_v = new RoadVertex(v1->getPt() * t + v2->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
		roads->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	// roads1を元に、エッジを登録していく
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		RoadVertexDesc src_desc = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt_desc = boost::target(*ei, roads1->graph);

		RoadVertexDesc new_src_desc = conv[src_desc];
		RoadVertexDesc new_tgt_desc = conv[tgt_desc];

		RoadEdge* new_e = new RoadEdge(1, 1);
		new_e->addPoint(roads->graph[new_src_desc]->getPt());
		new_e->addPoint(roads->graph[new_tgt_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(new_src_desc, new_tgt_desc, roads->graph);
		roads->graph[edge_pair1.first] = new_e;
	}

	return roads;
}

RoadGraph* Morph::buildGraph1() {
	RoadGraph* roads = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(120, 120));
	RoadVertexDesc v1_desc = boost::add_vertex(roads->graph);
	roads->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(280, 120));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(200, 280));
	RoadVertexDesc v3_desc = boost::add_vertex(roads->graph);
	roads->graph[v3_desc] = v3;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(v1->getPt());
	e1->addPoint(v2->getPt());
	std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(v1_desc, v2_desc, roads->graph);
	roads->graph[edge_pair1.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(v2->getPt());
	e2->addPoint(v3->getPt());
	std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(v2_desc, v3_desc, roads->graph);
	roads->graph[edge_pair2.first] = e2;

	RoadEdge* e3 = new RoadEdge(1, 1);
	e3->addPoint(v3->getPt());
	e3->addPoint(v1->getPt());
	std::pair<RoadEdgeDesc, bool> edge_pair3 = boost::add_edge(v3_desc, v1_desc, roads->graph);
	roads->graph[edge_pair3.first] = e3;

	return roads;
}

RoadGraph* Morph::buildGraph2() {
	RoadGraph* roads = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(120, 280));
	RoadVertexDesc v1_desc = boost::add_vertex(roads->graph);
	roads->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(200, 130));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(280, 280));
	RoadVertexDesc v3_desc = boost::add_vertex(roads->graph);
	roads->graph[v3_desc] = v3;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(v1->getPt());
	e1->addPoint(v2->getPt());
	e1->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(v1_desc, v2_desc, roads->graph);
	roads->graph[edge_pair1.first] = e1;
	
	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(v2->getPt());
	e2->addPoint(v3->getPt());
	e2->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(v2_desc, v3_desc, roads->graph);
	roads->graph[edge_pair2.first] = e2;

	return roads;
}

RoadGraph* Morph::buildGraph3() {
	RoadGraph* roads = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(200, 200));
	RoadVertexDesc v1_desc = boost::add_vertex(roads->graph);
	roads->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(100, 200));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(200, 100));
	RoadVertexDesc v3_desc = boost::add_vertex(roads->graph);
	roads->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(300, 200));
	RoadVertexDesc v4_desc = boost::add_vertex(roads->graph);
	roads->graph[v4_desc] = v4;

	RoadVertex* v5 = new RoadVertex(QVector2D(200, 300));
	RoadVertexDesc v5_desc = boost::add_vertex(roads->graph);
	roads->graph[v5_desc] = v5;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(v1->getPt());
	e1->addPoint(v2->getPt());
	e1->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(v1_desc, v2_desc, roads->graph);
	roads->graph[edge_pair1.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(v1->getPt());
	e2->addPoint(v3->getPt());
	e2->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(v1_desc, v3_desc, roads->graph);
	roads->graph[edge_pair2.first] = e2;

	RoadEdge* e3 = new RoadEdge(1, 1);
	e3->addPoint(v1->getPt());
	e3->addPoint(v4->getPt());
	e3->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair3 = boost::add_edge(v1_desc, v4_desc, roads->graph);
	roads->graph[edge_pair3.first] = e3;

	RoadEdge* e4 = new RoadEdge(1, 1);
	e4->addPoint(v1->getPt());
	e4->addPoint(v5->getPt());
	e4->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair4 = boost::add_edge(v1_desc, v5_desc, roads->graph);
	roads->graph[edge_pair4.first] = e4;

	return roads;
}

RoadGraph* Morph::buildGraph4() {
	RoadGraph* roads = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(200, 180));
	RoadVertexDesc v1_desc = boost::add_vertex(roads->graph);
	roads->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(100, 180));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(300, 180));
	RoadVertexDesc v3_desc = boost::add_vertex(roads->graph);
	roads->graph[v3_desc] = v3;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(v1->getPt());
	e1->addPoint(v2->getPt());
	e1->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(v1_desc, v2_desc, roads->graph);
	roads->graph[edge_pair1.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(v1->getPt());
	e2->addPoint(v3->getPt());
	e2->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(v1_desc, v3_desc, roads->graph);
	roads->graph[edge_pair2.first] = e2;

	return roads;
}

RoadGraph* Morph::buildGraph5() {
	RoadGraph* roads = new RoadGraph();

	RoadVertex* v1 = new RoadVertex(QVector2D(20, 120));
	RoadVertexDesc v1_desc = boost::add_vertex(roads->graph);
	roads->graph[v1_desc] = v1;

	RoadVertex* v2 = new RoadVertex(QVector2D(100, 200));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(200, 200));
	RoadVertexDesc v3_desc = boost::add_vertex(roads->graph);
	roads->graph[v3_desc] = v3;

	RoadVertex* v4 = new RoadVertex(QVector2D(280, 120));
	RoadVertexDesc v4_desc = boost::add_vertex(roads->graph);
	roads->graph[v4_desc] = v4;

	RoadEdge* e1 = new RoadEdge(1, 1);
	e1->addPoint(v1->getPt());
	e1->addPoint(v2->getPt());
	e1->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(v1_desc, v2_desc, roads->graph);
	roads->graph[edge_pair1.first] = e1;

	RoadEdge* e2 = new RoadEdge(1, 1);
	e2->addPoint(v3->getPt());
	e2->addPoint(v4->getPt());
	e2->orig = true;
	std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(v3_desc, v4_desc, roads->graph);
	roads->graph[edge_pair2.first] = e2;

	return roads;
}

/**
 * 各エッジに、指定した数のノードを追加する。
 */
void Morph::addNodesOnEdges(RoadGraph* roads, int numNodes) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* e = roads->graph[*ei];

		// もともとのエッジに、すべて「無効」のしるしをつける
		e->valid = false;
	}

	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* e = roads->graph[*ei];

		// 「無効」のしるしのついたエッジ（つまり、もともとのエッジ）に対してのみ、ノードを追加する。
		if (e->valid) continue;

		RoadVertexDesc src_desc = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt_desc = boost::target(*ei, roads->graph);

		RoadVertex* src = roads->graph[src_desc];
		RoadVertex* tgt = roads->graph[tgt_desc];
		
		RoadVertexDesc prev_v_desc = src_desc;
		for (int i = 0; i < numNodes; i++) {
			// ノードを追加
			RoadVertex* v = new RoadVertex(src->getPt() + (tgt->getPt() - src->getPt()) / (float)(numNodes + 1) * (float)(i + 1));
			v->orig = false;
			v->virt = true;		// 追加したノードは、暫定的に、完全仮想ノードとする。
			RoadVertexDesc v_desc = boost::add_vertex(roads->graph);
			roads->graph[v_desc] = v;

			// エッジを追加
			RoadEdge* new_e = new RoadEdge(1, 1);
			new_e->addPoint(roads->graph[prev_v_desc]->getPt());
			new_e->addPoint(roads->graph[v_desc]->getPt());
			std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(prev_v_desc, v_desc, roads->graph);
			roads->graph[new_e_pair.first] = new_e;
			
			prev_v_desc = v_desc;
		}

		// 最後のノードと、tgtの間にもエッジを追加
		RoadEdge* new_e = new RoadEdge(1, 1);
		new_e->addPoint(roads->graph[prev_v_desc]->getPt());
		new_e->addPoint(roads->graph[tgt_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(prev_v_desc, tgt_desc, roads->graph);
		roads->graph[new_e_pair.first] = new_e;			
	}

	// 「無効」のしるしのついたエッジ（つまり、もともとのエッジ）を全て削除する
	bool deleted = true;
	while (deleted) {
		deleted = false;
		for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
			RoadEdge* e = roads->graph[*ei];
			if (e->valid) continue;

			boost::remove_edge(*ei, roads->graph);
			deleted = true;
			break;
		}
	}
}

/**
 * 第１ステップ：Roads1の各頂点について、roads2の中から最も近い頂点を探し、対応関係をマップに入れて返却する。
 */
QMap<RoadVertexDesc, RoadVertexDesc> Morph::findNearestNeighbors(RoadGraph* roads1, RoadGraph* roads2) {
	QMap<RoadVertexDesc, RoadVertexDesc> neighbor1;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertexDesc v2_desc = findNearestNeighbor(roads2, roads1->graph[*vi]->getPt(), -1);
		neighbor1.insert(*vi, v2_desc);
	}

	return neighbor1;
}

/**
 * 第２ステップ：相思相愛のペアを全てマークする（pair = true）
 */
void Morph::checkExclusivePair(RoadGraph* roads, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (neighbor2->value(neighbor->value(*vi)) == *vi) {
			roads->graph[*vi]->pair = true;
		}
	}
}

/**
 * 第３ステップ：一人ぼっちの頂点について、相手がまだペアでない場合は、ペアになってもらう。
 */
void Morph::changeAloneToPair(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		// 一人ぼっちじゃない頂点は、スキップ
		if (roads1->graph[*vi]->pair) continue;

		// 相手が、既にペアになっている場合は、スキップ
		if (roads2->graph[neighbor1->value(*vi)]->pair) continue;

		// 相手に、強制的にペアになってもらう
		neighbor2->insert(neighbor1->value(*vi), *vi);
		roads1->graph[*vi]->pair = true;
		roads2->graph[neighbor1->value(*vi)]->pair = true;
	}
}

/**
 * 第４ステップ：roads1の一人ぼっち頂点に対し、相手頂点(roads2)を分裂させ、１対１対応になるようにする。
 * 
 */
void Morph::augmentGraph(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		// 一人ぼっちじゃない頂点は、スキップ
		if (roads1->graph[*vi]->pair) continue;

		// 相手頂点
		RoadVertexDesc v2_desc = neighbor1->value(*vi);
		RoadVertex* v2 = roads2->graph[v2_desc];

		// 相手頂点を分裂
		RoadVertex* v2_2 = new RoadVertex(v2->getPt());
		RoadVertexDesc v2_2_desc = boost::add_vertex(roads2->graph);
		roads2->graph[v2_2_desc] = v2_2;

		// 兄弟リストに追加
		std::vector<RoadVertexDesc> sibling;
		if (roads2->siblings.contains(v2_desc)) {
			sibling = roads2->siblings[v2_desc];
		} else {
			sibling.push_back(v2_desc);
		}
		sibling.push_back(v2_2_desc);

		// 対応関係を更新。
		neighbor1->insert(*vi, v2_2_desc);
		neighbor2->insert(v2_2_desc, *vi);

		// この結果、ペアになる。
		roads1->graph[*vi]->pair = true;
		roads2->graph[v2_2_desc]->pair = true;

		// 全兄弟に、更新した兄弟リストを設定
		for (int i = 0; i < sibling.size(); i++) {
			roads2->siblings[sibling[i]] = sibling;
		}

		// 兄弟間のエッジの更新
		for (int i = 0; i < sibling.size(); i++) {
			for (int j = i + 1; j < sibling.size(); j++) {
				// roads2の、兄弟頂点AとB
				RoadVertexDesc v2a = sibling[i];
				RoadVertexDesc v2b = sibling[j];

				// 対応するroads1の、兄弟頂点AとB
				RoadVertexDesc v1a = neighbor2->value(v2a);
				RoadVertexDesc v1b = neighbor2->value(v2b);

				// raods2の兄弟頂点AB間にエッジがすでにあれば、スキップ
				if (hasEdge(roads2, v2a, v2b)) continue;

				// 対応するroads1の、兄弟頂点AB間にエッジがあれば、roads2の兄弟頂点AB間にもエッジを作成する
				if (hasEdge(roads1, v1a, v1b)) {
					RoadEdge* new_e = new RoadEdge(1, 1);
					new_e->addPoint(roads2->graph[v2a]->getPt());
					new_e->addPoint(roads2->graph[v2b]->getPt());
					std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(v2a, v2b, roads2->graph);
					roads2->graph[new_e_pair.first] = new_e;
				}
			}
		}
	}
}

/**
 * 第５ステップ：エッジの対応関係をチェックする。
 * １）両方のグラフでエッジがある場合は、そのエッジの両端頂点をfinalizeする。
 * ２）相方のグラフにエッジがない場合、
 *     a) 両端頂点ABのうち、Aの兄弟の対応点A'と、Bの対応点B'の兄弟との間にエッジがないか、全ての兄弟について調べる。
 *        もしあれば、当該エッジを無効にし、代わりに、Aの兄弟とBの間にエッジを作成する。
 *        また、A'とB'の兄弟との間のエッジも無効にし、代わりに、A'とB'の間にエッジを作成する。
 *     b) まったく代替エッジがない場合、
 *        当該エッジの中間点に２つの頂点を追加し、相方グラフについては、
 *     　 対応するその両端頂点に２つの頂点を追加し、それらを対応させる。
 */
void Morph::updateEdges(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		// 当該エッジの両端頂点を取得
		RoadVertexDesc src_desc = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt_desc = boost::target(*ei, roads1->graph);
		RoadVertex* src = roads1->graph[src_desc];
		RoadVertex* tgt = roads1->graph[tgt_desc];

		// 対応頂点を取得
		RoadVertexDesc src2_desc = neighbor1->value(src_desc);
		RoadVertexDesc tgt2_desc = neighbor1->value(tgt_desc);
		RoadVertex* src2 = roads2->graph[src2_desc];
		RoadVertex* tgt2 = roads2->graph[tgt2_desc];
		
		// 対応グラフにもエッジがあれば、頂点をfinalizeする
		if (hasEdge(roads2, src2_desc, tgt2_desc)) {
			roads1->graph[src_desc]->finalized = true;
			roads1->graph[tgt_desc]->finalized = true;
			roads2->graph[src2_desc]->finalized = true;
			roads2->graph[tgt2_desc]->finalized = true;
			continue;
		}

		// 対策(a)に成功したら、対策(b)は不要。
		if (updateEdgeWithSibling1(roads1, neighbor1, roads2, neighbor2, src_desc, tgt_desc)) continue;
		if (updateEdgeWithSibling2(roads1, neighbor1, roads2, neighbor2, src_desc, tgt_desc)) continue;
		if (updateEdgeWithSibling4(roads1, neighbor1, roads2, neighbor2, src_desc, tgt_desc)) continue;
	}
}

/**
 * 第６ステップ：エッジの対応関係をチェックする。
 * １）両方のグラフでエッジがある場合は、そのエッジの両端頂点をfinalizeする。
 * ２）相方のグラフにエッジがない場合、
 *     b) まったく代替エッジがない場合、
 *        当該エッジの中間点に２つの頂点を追加し、相方グラフについては、
 *     　 対応するその両端頂点に２つの頂点を追加し、それらを対応させる。
 */
void Morph::updateEdges2(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		// 当該エッジの両端頂点を取得
		RoadVertexDesc src_desc = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt_desc = boost::target(*ei, roads1->graph);
		RoadVertex* src = roads1->graph[src_desc];
		RoadVertex* tgt = roads1->graph[tgt_desc];

		// 対応頂点を取得
		RoadVertexDesc src2_desc = neighbor1->value(src_desc);
		RoadVertexDesc tgt2_desc = neighbor1->value(tgt_desc);
		RoadVertex* src2 = roads2->graph[src2_desc];
		RoadVertex* tgt2 = roads2->graph[tgt2_desc];
		
		// 対応グラフにもエッジがあれば、頂点をfinalizeする
		if (hasEdge(roads2, src2_desc, tgt2_desc)) {
			roads1->graph[src_desc]->finalized = true;
			roads1->graph[tgt_desc]->finalized = true;
			roads2->graph[src2_desc]->finalized = true;
			roads2->graph[tgt2_desc]->finalized = true;
			continue;
		}

		// 対策(b)
		updateEdgeWithSplit(roads1, neighbor1, roads2, neighbor2, src_desc, tgt_desc);
	}
}

/**
 * 第５ステップの対策(a) - 1
 * roads1の頂点ABについて、兄弟を使って対応するエッジを見つけ、それに基づいてエッジを更新する。
 * 1) Aの兄弟の対応点A'と、Bの対応点B'の兄弟との間にエッジがないか、調べる。
 *    もしあれば、エッジABを無効にし、代わりに、Aの兄弟とBの間にエッジを作成する。
 *    また、A'とB'の兄弟との間のエッジも無効にし、代わりに、A'とB'の間にエッジを作成する。
 */
bool Morph::updateEdgeWithSibling1(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt) {
	bool matched = false;

	if (roads1->siblings.contains(src)) {
		// Bの対応点B'を取得
		RoadVertexDesc t = neighbor1->value(tgt);

		// B'の兄弟がいる場合、
		if (roads2->siblings.contains(t)) {
			for (int i = 0; i < roads1->siblings[src].size(); i++) {
				// Aの兄弟を取得
				RoadVertexDesc s = roads1->siblings[src][i];
				if (s == src) continue;

				// Aの兄弟の対応点A'を取得
				RoadVertexDesc s2 = neighbor1->value(s);

				for (int j = 0; j < roads2->siblings[t].size(); j++) {
					// B'の兄弟を取得
					RoadVertexDesc t2 = roads2->siblings[t][j];

					if (hasOriginalEdge(roads2, s2, t2)) {
						// エッジABを無効にする
						RoadEdgeDesc e1 = getEdge(roads1, src, tgt);
						roads1->graph[e1]->valid = false;

						// Aの兄弟とBの間にエッジを作成
						RoadEdge* new_e1 = new RoadEdge(1, 1);
						new_e1->addPoint(roads1->graph[s]->getPt());
						new_e1->addPoint(roads1->graph[tgt]->getPt());
						std::pair<RoadEdgeDesc, bool> new_e1_pair = boost::add_edge(s, tgt, roads1->graph);
						roads1->graph[new_e1_pair.first] = new_e1;

						if (t != t2) {
							// A'とB'の兄弟との間のエッジを無効にする
							RoadEdgeDesc e2 = getEdge(roads2, s2, t2);
							roads2->graph[e2]->valid = false;

							// A'とB'の間にエッジを作成する
							RoadEdge* new_e2 = new RoadEdge(1, 1);
							new_e2->addPoint(roads2->graph[s2]->getPt());
							new_e2->addPoint(roads2->graph[t]->getPt());
							std::pair<RoadEdgeDesc, bool> new_e2_pair = boost::add_edge(s2, t, roads2->graph);
							roads2->graph[new_e2_pair.first] = new_e2;
						}

						matched = true;
						break;
					}
				}

				if (matched) break;
			}
		}
	}

	if (!matched && roads1->siblings.contains(tgt)) {
		// Aの対応点A'を取得
		RoadVertexDesc s = neighbor1->value(src);

		// A'の兄弟がいる場合、
		if (roads2->siblings.contains(s)) {
			for (int i = 0; i < roads1->siblings[tgt].size(); i++) {
				// Bの兄弟を取得
				RoadVertexDesc t = roads1->siblings[tgt][i];
				if (t == tgt) continue;

				// Bの兄弟の対応点B'を取得
				RoadVertexDesc t2 = neighbor1->value(t);

				for (int j = 0; j < roads2->siblings[s].size(); j++) {
					// A'の兄弟を取得
					RoadVertexDesc s2 = roads2->siblings[s][j];
					//if (s2 == s) continue;

					if (hasOriginalEdge(roads2, s2, t2)) {
						// エッジABを無効にする
						RoadEdgeDesc e1 = getEdge(roads1, src, tgt);
						roads1->graph[e1]->valid = false;

						// AとBの兄弟の間にエッジを作成
						RoadEdge* new_e1 = new RoadEdge(1, 1);
						new_e1->addPoint(roads1->graph[src]->getPt());
						new_e1->addPoint(roads1->graph[t]->getPt());
						std::pair<RoadEdgeDesc, bool> new_e1_pair = boost::add_edge(src, t, roads1->graph);
						roads1->graph[new_e1_pair.first] = new_e1;

						if (s != s2) {
							// A'の兄弟とB'との間のエッジを無効にする
							RoadEdgeDesc e2 = getEdge(roads2, s2, t2);
							roads2->graph[e2]->valid = false;

							// A'とB'の間にエッジを作成する
							RoadEdge* new_e2 = new RoadEdge(1, 1);
							new_e2->addPoint(roads2->graph[s]->getPt());
							new_e2->addPoint(roads2->graph[t2]->getPt());
							std::pair<RoadEdgeDesc, bool> new_e2_pair = boost::add_edge(s, t2, roads2->graph);
							roads2->graph[new_e2_pair.first] = new_e2;
						}

						matched = true;
						break;
					}
				}

				if (matched) break;
			}
		}
	}

	return matched;
}

/**
 * 第５ステップの対策(a) - 2
 * roads1の頂点ABについて、兄弟を使って対応するエッジを見つけ、それに基づいてエッジを更新する。
 * 2) Aの兄弟の対応点A'と、Bの兄弟の対応点B'との間にエッジがないか、調べる。
 *    もしあれば、エッジABを無効にし、代わりに、Aの兄弟とBの兄弟の間にエッジを作成する。
 */
bool Morph::updateEdgeWithSibling2(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt) {
	bool matched = false;

	// AまたはBに兄弟がいない場合は、失敗
	if (!roads1->siblings.contains(src) || !roads1->siblings.contains(tgt)) return false;

	for (int i = 0; i < roads1->siblings[src].size(); i++) {
		// Aの兄弟を取得
		RoadVertexDesc s = roads1->siblings[src][i];
		//if (s == src) continue;

		// Aの兄弟の対応点A'を取得
		RoadVertexDesc s2 = neighbor1->value(s);

		for (int j = 0; j < roads1->siblings[tgt].size(); j++) {
			// Bの兄弟を取得
			RoadVertexDesc t = roads1->siblings[tgt][j];
			//if (t == tgt) continue;

			// Aの兄弟がAで、Bの兄弟がBなら、意味が無いのでスキップ。
			if (s == src && t == tgt) continue;

			// Bの兄弟の対応点B'を取得
			RoadVertexDesc t2 = neighbor1->value(t);

			if (hasOriginalEdge(roads2, s2, t2)) {
				// エッジABを無効にする
				RoadEdgeDesc e1 = getEdge(roads1, src, tgt);
				roads1->graph[e1]->valid = false;

				// Aの兄弟とBの兄弟の間にエッジを作成
				RoadEdge* new_e1 = new RoadEdge(1, 1);
				new_e1->addPoint(roads1->graph[s]->getPt());
				new_e1->addPoint(roads1->graph[t]->getPt());
				std::pair<RoadEdgeDesc, bool> new_e1_pair = boost::add_edge(s, t, roads1->graph);
				roads1->graph[new_e1_pair.first] = new_e1;

				matched = true;
				break;
			}
		}

		if (matched) break;
	}

	return matched;
}

/**
 * 第５ステップの対策(a) - 3
 * roads1の頂点ABについて、兄弟を使って対応するエッジを見つけ、それに基づいてエッジを更新する。
 * 3) Aの対応点A'の兄弟と、Bの兄弟の対応点B'との間にエッジがないか、調べる。
 *    もしあれば、エッジABを無効にし、代わりに、AとBの兄弟の間にエッジを作成する。
 *    また、A'の兄弟と、B'との間のエッジを無効にし、代わりに、A'とB'の間にエッジを作成する。
 */
bool Morph::updateEdgeWithSibling3(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt) {
	bool matched = false;

	// 対応点A'を取得
	RoadVertexDesc s = neighbor1->value(src);

	// 対応点A'の兄弟について
	for (int i = 0; i < roads2->siblings[s].size(); i++) {
		// A'の兄弟を取得
		RoadVertexDesc s2 = roads2->siblings[s][i];
		if (s2 == s) continue;

		for (int j = 0; j < roads1->siblings[tgt].size(); j++) {
			// Bの兄弟を取得
			RoadVertexDesc t = roads1->siblings[tgt][j];

			// Bの兄弟の対応点B'を取得
			RoadVertexDesc t2 = neighbor1->value(t);

			if (hasOriginalEdge(roads2, s2, t2)) {
				// A'の兄弟とB'の間のエッジを無効にする
				RoadEdgeDesc e2 = getEdge(roads2, s2, t2);
				roads2->graph[e2]->valid = false;

				// A'とB'の間にエッジを作成
				RoadEdge* new_e2 = new RoadEdge(1, 1);
				new_e2->addPoint(roads2->graph[s]->getPt());
				new_e2->addPoint(roads2->graph[t2]->getPt());
				std::pair<RoadEdgeDesc, bool> new_e2_pair = boost::add_edge(s, t2, roads2->graph);
				roads2->graph[new_e2_pair.first] = new_e2;

				// エッジABを無効にする
				RoadEdgeDesc e1 = getEdge(roads1, src, tgt);
				roads1->graph[e1]->valid = false;

				// AとBの兄弟の間にエッジを作成する
				RoadEdge* new_e1 = new RoadEdge(1, 1);
				new_e1->addPoint(roads1->graph[src]->getPt());
				new_e1->addPoint(roads1->graph[t]->getPt());
				std::pair<RoadEdgeDesc, bool> new_e1_pair = boost::add_edge(src, t, roads1->graph);
				roads1->graph[new_e1_pair.first] = new_e1;

				matched = true;
				break;
			}
		}

		if (matched) break;
	}

	return matched;
}

/**
 * 第５ステップの対策(a) - 4
 * roads1の頂点ABについて、兄弟を使って対応するエッジを見つけ、それに基づいてエッジを更新する。
 * 4) Aの対応点A'の兄弟と、Bの対応点B'の兄弟との間にエッジがないか、調べる。
 *    もしあれば、そのエッジを無効にし、代わりに、A'とB'の間にエッジを作成する。
 */
bool Morph::updateEdgeWithSibling4(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src, RoadVertexDesc tgt) {
	bool matched = false;

	// 対応点A', B'を取得
	RoadVertexDesc s = neighbor1->value(src);
	RoadVertexDesc t = neighbor1->value(tgt);

	// 対応点A'の兄弟について
	for (int i = 0; i < roads2->siblings[s].size(); i++) {
		// A'の兄弟を取得
		RoadVertexDesc s2 = roads2->siblings[s][i];

		for (int j = 0; j < roads2->siblings[t].size(); j++) {
			// Bの兄弟を取得
			RoadVertexDesc t2 = roads2->siblings[t][j];

			// A'の兄弟がA'で、B'の兄弟がB'なら、意味が無いのでスキップ。
			if (s == s2 && t == t2) continue;

			if (hasOriginalEdge(roads2, s2, t2)) {
				// A'の兄弟とB'の兄弟の間のエッジを無効にする
				RoadEdgeDesc e1 = getEdge(roads2, s2, t2);
				roads2->graph[e1]->valid = false;

				// A'とB'の間にエッジを作成
				RoadEdge* new_e = new RoadEdge(1, 1);
				new_e->addPoint(roads2->graph[s]->getPt());
				new_e->addPoint(roads2->graph[t]->getPt());
				std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(s, t, roads2->graph);
				roads2->graph[new_e_pair.first] = new_e;

				matched = true;
				break;
			}
		}

		if (matched) break;
	}

	return matched;
}

/**
 * roads1の頂点ABについて、対応グラフにまったく対応するエッジがない場合、エッジを分割して対応する。
 * エッジABの中間点に２つの頂点A2, B2を追加し、相方グラフについては、ABに対応する頂点A'B'にそれぞれ頂点A2', B2'を追加し、
 * A2とA2'、B2とB2'をそれぞれ対応させる。
 * また、A2-B2間にはエッジを作成するが、A2'-B2'間にはエッジを作成しない。
 */
void Morph::updateEdgeWithSplit(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2, RoadVertexDesc src_desc, RoadVertexDesc tgt_desc) {
	RoadVertex* src = roads1->graph[src_desc];
	RoadVertex* tgt = roads1->graph[tgt_desc];

	// 頂点A, Bの対応点A', B'を取得
	RoadVertexDesc src2_desc = neighbor1->value(src_desc);
	RoadVertexDesc tgt2_desc = neighbor1->value(tgt_desc);
	RoadVertex* src2 = roads2->graph[src2_desc];
	RoadVertex* tgt2 = roads2->graph[tgt2_desc];

	// roads1の当該エッジの中間点に頂点２つ追加
	RoadVertex* new_v1a = new RoadVertex((src->getPt() + tgt->getPt()) / 2.0f);
	RoadVertex* new_v1b = new RoadVertex((src->getPt() + tgt->getPt()) / 2.0f);
	RoadVertexDesc new_v1a_desc = boost::add_vertex(roads1->graph);
	roads1->graph[new_v1a_desc] = new_v1a;
	RoadVertexDesc new_v1b_desc = boost::add_vertex(roads1->graph);
	roads1->graph[new_v1b_desc] = new_v1b;

	// roads1の追加頂点に対してエッジを追加
	RoadEdge* new_e1a = new RoadEdge(1, 1);
	new_e1a->addPoint(src->getPt());
	new_e1a->addPoint(new_v1a->getPt());
	std::pair<RoadEdgeDesc, bool> new_e1a_pair = boost::add_edge(src_desc, new_v1a_desc, roads1->graph);
	roads1->graph[new_e1a_pair.first] = new_e1a;

	RoadEdge* new_e1b = new RoadEdge(1, 1);
	new_e1b->addPoint(new_v1b->getPt());
	new_e1b->addPoint(tgt->getPt());
	std::pair<RoadEdgeDesc, bool> new_e1b_pair = boost::add_edge(new_v1b_desc, tgt_desc, roads1->graph);
	roads1->graph[new_e1b_pair.first] = new_e1b;

	// roads1の当該エッジを無効にする
	RoadEdgeDesc e = getEdge(roads1, src_desc, tgt_desc);
	roads1->graph[e]->valid = false;

	// roads2の対応頂点に、頂点２つ追加
	RoadVertex* new_v2a = new RoadVertex(src2->getPt());
	RoadVertex* new_v2b = new RoadVertex(tgt2->getPt());
	RoadVertexDesc new_v2a_desc = boost::add_vertex(roads2->graph);
	roads2->graph[new_v2a_desc] = new_v2a;
	RoadVertexDesc new_v2b_desc = boost::add_vertex(roads2->graph);
	roads2->graph[new_v2b_desc] = new_v2b;

	// roads2の追加頂点に対してエッジを追加
	RoadEdge* new_e2a = new RoadEdge(1, 1);
	new_e2a->addPoint(src2->getPt());
	new_e2a->addPoint(new_v2a->getPt());
	std::pair<RoadEdgeDesc, bool> new_e2a_pair = boost::add_edge(src2_desc, new_v2a_desc, roads2->graph);
	roads2->graph[new_e2a_pair.first] = new_e2a;

	RoadEdge* new_e2b = new RoadEdge(1, 1);
	new_e2b->addPoint(new_v2b->getPt());
	new_e2b->addPoint(tgt2->getPt());
	std::pair<RoadEdgeDesc, bool> new_e2b_pair = boost::add_edge(new_v2b_desc, tgt2_desc, roads2->graph);
	roads2->graph[new_e2b_pair.first] = new_e2b;

	// 対応点を更新
	neighbor1->insert(new_v1a_desc, new_v2a_desc);
	neighbor1->insert(new_v1b_desc, new_v2b_desc);
	neighbor2->insert(new_v2a_desc, new_v1a_desc);
	neighbor2->insert(new_v2b_desc, new_v1b_desc);
}

/**
 * 対象グラフの中から、指定した点に最も近い頂点descを返却する。
 * ただし、ignore頂点は、検索対象から外す。
 */
RoadVertexDesc Morph::findNearestNeighbor(RoadGraph* roads, QVector2D pt, RoadVertexDesc ignore) {
	RoadVertexDesc nearest_desc;
	float min_dist = std::numeric_limits<float>::max();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (*vi == ignore) continue;

		float dist = (roads->graph[*vi]->getPt() - pt).length();
		if (dist < min_dist) {
			nearest_desc = *vi;
			min_dist = dist;
		}
	}

	return nearest_desc;
}

/**
 * ２つの頂点間にエッジがあるかチェックする。
 * ただし、無効フラグの立っているエッジは、エッジがないとみなす。
 */
bool Morph::hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		if (src == desc1 && tgt == desc2) return true;
		if (tgt == desc1 && src == desc2) return true;
	}

	return false;
}

/**
 * ２つの頂点間にエッジがあるか、または、もともとエッジがあったかチェックする。
 */
bool Morph::hasOriginalEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid && !roads->graph[*ei]->orig) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		if (src == desc1 && tgt == desc2) return true;
		if (tgt == desc1 && src == desc2) return true;
	}

	return false;
}

RoadEdgeDesc Morph::getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt) {
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(src, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid && !roads->graph[*ei]->orig) continue;

		if (boost::target(*ei, roads->graph) == tgt) return *ei;
	}

	throw "No edge found.";
}

void Morph::setupSiblings(RoadGraph* roads) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->siblings.contains(*vi)) {
			std::vector<RoadVertexDesc> sibling;
			sibling.push_back(*vi);
			roads->siblings[*vi] = sibling;
		}
	}
}