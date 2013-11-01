#include "Morph.h"
#include <qset.h>
#include <qpainter.h>
#include <qcolor.h>

Morph::Morph(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	timer = new QTimer(this);

	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	roads = NULL;
	roads1 = NULL;
	roads2 = NULL;
	new_roads1 = NULL;
	new_roads2 = NULL;
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	drawGraph(&painter, roads1, QColor(0, 0, 255));
	drawGraph(&painter, roads2, QColor(255, 0, 0));
	drawRelation(&painter, roads1, neighbor1, roads2, neighbor2);
}

void Morph::drawGraph(QPainter *painter, RoadGraph *roads, QColor col) {
	if (roads == NULL) return;

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(QPen(col, 2, Qt::SolidLine, Qt::RoundCap));
	painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* edge = roads->graph[*ei];

		for (int i = 0; i < edge->getPolyLine().size() - 1; i++) {
			painter->drawLine(edge->getPolyLine()[i].x(), edge->getPolyLine()[i].y(), edge->getPolyLine()[i+1].x(), edge->getPolyLine()[i+1].y());
		}
	}

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		RoadVertex* v = roads->graph[*vi];
		if (v->virt) continue;

		painter->fillRect((int)v->getPt().x() - 3, (int)v->getPt().y() - 3, 6, 6, col);
	}
}

void Morph::drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2) {
	if (roads1 == NULL || roads2 == NULL) return;

	painter->setPen(QPen(Qt::black, 2, Qt::DotLine, Qt::RoundCap));

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roads1->graph[*vi];
		RoadVertex* v2 = roads2->graph[neighbor1[*vi]];

		if (v1->virt || v2->virt) continue;

		painter->drawLine(v1->getPt().x(), v1->getPt().y(), v2->getPt().x(), v2->getPt().y());
	}
}

void Morph::start() {
	if (roads1 != NULL) {
		delete roads1;
	}
	if (roads2 != NULL) {
		delete roads2;
	}
	roads1 = buildGraph1();
	roads2 = buildGraph2();
	//buildCorrespondence();
	/*
	buildCorrespondence(roads1, roads2);

	for (int i = 0; i < 3; i++) {
		int a = corr1[i];
		int b = 0;
	}
	for (int i = 0; i < 3; i++) {
		int a = corr2[i];
		int b = 0;
	}
	*/

	addNodesOnEdges(roads1, 10);
	addNodesOnEdges(roads2, 10);
	findNearestNeighbors(roads1, &neighbor1, roads2, &neighbor2);
	findNearestNeighbors(roads2, &neighbor2, roads1, &neighbor1);

	QMap<RoadVertexDesc, QVector2D> n1;
	QMap<RoadVertexDesc, QVector2D> n2;

	// 対応関係を、一旦座標データに変換する
	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = neighbor1.begin(); it != neighbor1.end(); ++it) {
		RoadVertexDesc v1 = it.key();
		RoadVertexDesc v2 = neighbor1[v1];
		n1[v1] = roads2->graph[v2]->getPt();
	}
	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = neighbor2.begin(); it != neighbor2.end(); ++it) {
		RoadVertexDesc v1 = it.key();
		RoadVertexDesc v2 = neighbor2[v1];
		n2[v1] = roads1->graph[v2]->getPt();
	}

	// 対応関係を構築したので、完全仮想頂点を全て削除する
	removeVirtVertices(roads1);
	removeVirtVertices(roads2);

	// 座標データに基づいて、対応関係を復旧する。
	neighbor1.clear();
	for (QMap<RoadVertexDesc, QVector2D>::iterator it = n1.begin(); it != n1.end(); ++it) {
		RoadVertexDesc v1 = it.key();
		RoadVertexDesc v2 = findNearestNeighbor(roads2, n1[v1], -1);
		neighbor1[v1] = v2;
	}
	neighbor2.clear();
	for (QMap<RoadVertexDesc, QVector2D>::iterator it = n2.begin(); it != n2.end(); ++it) {
		RoadVertexDesc v1 = it.key();
		RoadVertexDesc v2 = findNearestNeighbor(roads1, n2[v1], -1);
		neighbor2[v1] = v2;
	}



	checkEdges(roads1, &neighbor1, roads2, &neighbor2);
	checkEdges(roads2, &neighbor2, roads1, &neighbor1);

	/*
	augmentGraph();
	findExclusiveNearestNeighbor(roads1, roads2);
	buildEdges(roads1, roads2);
	*/

	t = 0.0f;

	timer->start(200);
}

void Morph::tick() {
	/*
	if (roads != NULL) {
		delete roads;
	}
	roads = interpolate(roads1, roads2, t);
	*/

	update();

	t += 0.02f;
	if (t > 1.0f) timer->stop();
}

RoadGraph* Morph::interpolate(RoadGraph* roads1, RoadGraph* roads2, float t) {
	RoadGraph* roads = new RoadGraph();

	/*
	QSet<RoadVertexDesc> finishedRoads2;
	QMap<RoadVertexDesc, RoadVertexDesc> conv1;
	QMap<RoadVertexDesc, RoadVertexDesc> conv2;

	// add vertices from roads1
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertexDesc v2 = corr1[*vi];

		RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt() * t + roads2->graph[v2]->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
		roads->graph[new_v_desc] = new_v;

		conv1[*vi] = new_v_desc;
		conv2[v2] = new_v_desc;
		finishedRoads2.insert(v2);
	}

	// add vertices from roads2 if they are not added
	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		if (finishedRoads2.contains(*vi)) continue;

		RoadVertexDesc v1 = corr2[*vi];

		RoadVertex* new_v = new RoadVertex(roads1->graph[v1]->getPt() * t + roads2->graph[*vi]->getPt() * (1 - t));
		RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
		roads->graph[new_v_desc] = new_v;

		conv2[*vi] = new_v_desc;
	}

	// add edges based on roads1
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads1->graph);

		RoadVertexDesc new_src = conv1[src];
		RoadVertexDesc new_tgt = conv1[tgt];

		RoadEdge* new_e = new RoadEdge(1, 1);
		new_e->addPoint(roads->graph[new_src]->getPt());
		new_e->addPoint(roads->graph[new_tgt]->getPt());
		std::pair<RoadEdgeDesc, bool> edge_pair1 = boost::add_edge(new_src, new_tgt, roads->graph);
		roads->graph[edge_pair1.first] = new_e;
	}

	// add edges based on roads2
	for (boost::tie(ei, eend) = boost::edges(roads2->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads2->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads2->graph);

		RoadVertexDesc new_src = conv2[src];
		RoadVertexDesc new_tgt = conv2[tgt];

		RoadEdge* new_e = new RoadEdge(1, 1);
		new_e->addPoint(roads->graph[new_src]->getPt());
		new_e->addPoint(roads->graph[new_tgt]->getPt());
		std::pair<RoadEdgeDesc, bool> edge_pair2 = boost::add_edge(new_src, new_tgt, roads->graph);
		roads->graph[edge_pair2.first] = new_e;
	}
	*/

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

	RoadVertex* v2 = new RoadVertex(QVector2D(200, 120));
	RoadVertexDesc v2_desc = boost::add_vertex(roads->graph);
	roads->graph[v2_desc] = v2;

	RoadVertex* v3 = new RoadVertex(QVector2D(280, 280));
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

	return roads;
}

void Morph::buildCorrespondence(RoadGraph* roads1, RoadGraph* roads2) {
	corr1.clear();
	corr2.clear();

	/*
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* vertex = roads1->graph[*vi];

		RoadVertexDesc nearest_desc;
		float min_dist = std::numeric_limits<float>::max();

		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(roads2->graph); vi2 != vend2; ++vi2) {
			float dist = (roads2->graph[*vi2]->getPt() - vertex->getPt()).length();
			if (dist < min_dist) {
				nearest_desc = *vi2;
				min_dist = dist;
			}
		}

		corr1[*vi] = nearest_desc;
	}

	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		RoadVertex* vertex = roads2->graph[*vi];

		RoadVertexDesc nearest_desc;
		float min_dist = std::numeric_limits<float>::max();

		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(roads1->graph); vi2 != vend2; ++vi2) {
			float dist = (roads1->graph[*vi2]->getPt() - vertex->getPt()).length();
			if (dist < min_dist) {
				nearest_desc = *vi2;
				min_dist = dist;
			}
		}

		corr2[*vi] = nearest_desc;
	}

	*/
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
			roads1->graph[new_e_pair.first] = new_e;
			
			prev_v_desc = v_desc;
		}

		// 最後のノードと、tgtの間にもエッジを追加
		RoadEdge* new_e = new RoadEdge(1, 1);
		new_e->addPoint(roads->graph[prev_v_desc]->getPt());
		new_e->addPoint(roads->graph[tgt_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(prev_v_desc, tgt_desc, roads->graph);
		roads1->graph[new_e_pair.first] = new_e;			
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
 * Roads1の各頂点について、roads2の中から最も近い頂点を探し、対応関係をマップに入れて返却する。
 */
void Morph::findNearestNeighbors(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor2) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		// 完全仮想頂点は、対応関係を作成しない。
		if (roads1->graph[*vi]->virt) continue;

		// 対応関係がすでに作成済みの場合は、スキップ
		if (neighbor1->contains(*vi)) continue;

		RoadVertexDesc v2_desc = findNearestNeighbor(roads2, roads1->graph[*vi]->getPt(), -1);
		neighbor1->insert(*vi, v2_desc);
		neighbor2->insert(v2_desc, *vi);

		roads2->graph[v2_desc]->virt = false;	// 対応関係が発生したので、本当の頂点として扱う
	}
}

/**
 * roads1の完全仮想頂点を全て削除する。
 */
void Morph::removeVirtVertices(RoadGraph* roads) {
	bool deleted = true;
	while (deleted) {
		deleted = false;

		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
			if (roads->graph[*vi]->virt == false) continue;

			// 完全仮想頂点なので、削除する

			// まず、隣接２頂点を取得する。
			RoadVertexDesc desc[2];
			RoadEdge* e[2];
			int count = 0;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, roads->graph); oei != oeend; ++oei, ++count) {
				e[count] = roads->graph[*oei];
				desc[count] = boost::target(*oei, roads->graph);
			}

			// 隣接２頂点間にエッジを作成
			RoadEdge* new_e = new RoadEdge(1, 1);
			new_e->addPoint(roads->graph[desc[0]]->getPt());
			new_e->addPoint(roads->graph[desc[1]]->getPt());
			std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(desc[0], desc[1], roads->graph);
			roads->graph[new_e_pair.first] = new_e;

			// エッジを削除
			boost::remove_edge(desc[0], *vi, roads->graph);
			boost::remove_edge(desc[1], *vi, roads->graph);
			delete e[0];
			if (e[1] != e[0]) delete e[1];

			// ノードを削除
			boost::remove_vertex(*vi, roads->graph);

			deleted = true;
			break;
		}
	}
}

/**
 * Roads1の各エッジについて、両端２頂点ABに対応する２頂点CD間にエッジがあることを確認する。
 * ない場合は、対応する２頂点CDのうち、１つの頂点Cを２つに分割し、A-C、B-C'を対応関係とする。
 */
void Morph::checkEdges(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> *neighbor1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>* neighbor2) {
	bool updated = true;

	while (updated) {
		updated = false;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
			// 両端２頂点AB
			RoadVertexDesc src_desc = boost::source(*ei, roads1->graph);
			RoadVertexDesc tgt_desc = boost::target(*ei, roads1->graph);

			// 頂点が完全仮想頂点の場合は、スキップ
			if (roads1->graph[src_desc]->virt || roads1->graph[tgt_desc]->virt) continue;

			// 対応する２頂点CD
			RoadVertexDesc src_desc2 = neighbor1->value(src_desc);
			RoadVertexDesc tgt_desc2 = neighbor1->value(tgt_desc);

			// CD間にエッジがあれば、めでたし、めでたし
			if (hasExclusiveEdge(roads2, src_desc2, tgt_desc2)) continue;

			if (roads2->graph[src_desc2]->orig) {
				// Cを２つに分割し、A-C、B-C'の対応関係にする。
				RoadVertex* new_v = new RoadVertex(roads2->graph[src_desc2]->getPt());
				new_v->orig = roads2->graph[src_desc2]->orig;
				new_v->virt = roads2->graph[src_desc2]->virt;
				RoadVertexDesc new_v_desc = boost::add_vertex(roads2->graph);
				roads2->graph[new_v_desc] = new_v;

				// B-C'の対応関係を作成
				neighbor1->insert(tgt_desc, new_v_desc);
				neighbor2->insert(new_v_desc, tgt_desc);

				// Dの対応関係は不要になったので、削除する
				neighbor2->remove(tgt_desc2);

				// Dは、完全仮想頂点となる。
				roads2->graph[tgt_desc2]->virt = true;
			} else {
				// Dを２つに分割し、B-D, A-D'の関係とする。
				RoadVertex* new_v = new RoadVertex(roads2->graph[tgt_desc2]->getPt());
				new_v->orig = roads2->graph[tgt_desc2]->orig;
				new_v->virt = roads2->graph[tgt_desc2]->virt;
				RoadVertexDesc new_v_desc = boost::add_vertex(roads2->graph);
				roads2->graph[new_v_desc] = new_v;

				// A-D'の対応関係を作成
				neighbor1->insert(src_desc, new_v_desc);
				neighbor2->insert(new_v_desc, src_desc);

				// Cの対応関係は不要になったので、削除する
				neighbor2->remove(src_desc2);

				// Cは、完全仮想頂点となる。
				roads2->graph[src_desc2]->virt = true;
			}

			updated = true;
			break;
		}
	}
}

/**
 * Nearest neighborに基づいてグラフを更新する。
 * 複数のノードからNearest neighborとして参照されているノードは、その分だけ、数を増やす。
 */
void Morph::augmentGraph() {
	// 参照カウントを0に初期化する。
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		roads1->graph[*vi]->ref = 0;
	}
	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		roads2->graph[*vi]->ref = 0;
	}

	// 相手グラフのNearest neighborデータに基づき、参照カウントを更新する.
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		roads2->graph[neighbor1[*vi]]->ref++;
	}
	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		roads1->graph[neighbor2[*vi]]->ref++;
	}

	//sibling1.clear();
	//sibling2.clear();

	// ここで、対応関係を表示してデバッグしたい。
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

	// 参照カウントに基づいて、グラフ１のノードを増やす
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		int aaa = *vi;
		if (roads1->graph[*vi]->ref > 1) {
			std::vector<RoadVertexDesc> sib;
			sib.push_back(*vi);

			for (int i = 0; i < roads1->graph[*vi]->ref - 1; i++) {
				RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt());
				RoadVertexDesc new_v_desc = boost::add_vertex(roads1->graph);
				roads1->graph[new_v_desc] = new_v;
				sib.push_back(new_v_desc);
			}

			for (int i = 0; i < sib.size(); i++) {
				roads1->sibling[sib[i]] = sib;
			}
		} else {
			if (!roads1->sibling.contains(*vi)) {
				std::vector<RoadVertexDesc> sib;
				sib.push_back(*vi);
				roads1->sibling[*vi] = sib;
			}
		}
	}

	// 参照カウントに基づいて、グラフ２のノードを増やす
	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		int aaa = *vi;
		if (roads2->graph[*vi]->ref > 1) {
			std::vector<RoadVertexDesc> sib;
			sib.push_back(*vi);

			for (int i = 0; i < roads2->graph[*vi]->ref - 1; i++) {
				RoadVertex* new_v = new RoadVertex(roads2->graph[*vi]->getPt());
				RoadVertexDesc new_v_desc = boost::add_vertex(roads2->graph);
				roads2->graph[new_v_desc] = new_v;
				sib.push_back(new_v_desc);
			}

			for (int i = 0; i < sib.size(); i++) {
				roads2->sibling[sib[i]] = sib;
			}
		} else {
			if (!roads2->sibling.contains(*vi)) {
				std::vector<RoadVertexDesc> sib;
				sib.push_back(*vi);
				roads2->sibling[*vi] = sib;
			}
		}
	}
}

/**
 * 参照カウントに基づいてノードを増やすと共に、現在のNearest neighborデータを更新して、
 * 完全に１対１の対応関係になるようにする。
 */
void Morph::findExclusiveNearestNeighbor(RoadGraph* roads1, RoadGraph* roads2) {
	QSet<RoadVertexDesc> invalidList1;
	QSet<RoadVertexDesc> invalidList2;

	// 参照カウントと兄弟データを使って、対応関係を１対１になるよう更新する。
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		int aaa = *vi;
		if (roads2->graph[neighbor1[*vi]]->ref > 1) {
			invalidList2.insert(neighbor1[*vi]);	// 対応が変わったので、対応相手のデータは無効になる
			roads2->graph[neighbor1[*vi]]->ref--;

			int bbb = findNearestNeighbor(roads2, roads1->graph[*vi]->getPt(), neighbor1[*vi]);
			neighbor1[*vi] = bbb;
			neighbor2[neighbor1[*vi]] = *vi;
		}
	}

	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		int aaa = *vi;
		if (roads1->graph[neighbor2[*vi]]->ref > 1) {
			invalidList1.insert(neighbor2[*vi]);	// 対応が変わったので、対応相手のデータは無効になる
			roads1->graph[neighbor2[*vi]]->ref--;

			int bbb = findNearestNeighbor(roads1, roads2->graph[*vi]->getPt(), neighbor2[*vi]);
			neighbor2[*vi] = bbb;
			neighbor1[neighbor2[*vi]] = *vi;
		}
	}

	// ここで、対応関係を表示してデバッグしたい。
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

	for (QSet<RoadVertexDesc>::iterator it = invalidList1.begin(); it != invalidList1.end(); ++it) {
		int aaa = *it;
		int ccc = 0;
	}
	for (QSet<RoadVertexDesc>::iterator it = invalidList2.begin(); it != invalidList2.end(); ++it) {
		int aaa = *it;
		int ccc = 0;
	}

	// 無効になった隣接リストを更新する。
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (invalidList1.contains(*vi)) continue;

		neighbor2[neighbor1[*vi]] = *vi;
	}

	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		if (invalidList2.contains(*vi)) continue;

		neighbor1[neighbor2[*vi]] = *vi;
	}

	// ここで、対応関係を表示してデバッグしたい。
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
 * 既存のグラフデータなどを使って、エッジをつけていく
 */
void Morph::buildEdges(RoadGraph* roads1, RoadGraph* roads2) {
	new_roads1 = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> convV1;
	QMap<RoadVertexDesc, RoadVertexDesc> convV2;

	// roads1からnew_roads1へ、頂点をコピーする
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertex* v = roads1->graph[*vi];

		RoadVertex* new_v = new RoadVertex(v->getPt());
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads1->graph);
		new_roads1->graph[new_v_desc] = new_v;

		convV1[*vi] = new_v_desc;
	}

	// roads2からnew_roads2へ、頂点をコピーする
	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		RoadVertex* v = roads2->graph[*vi];

		RoadVertex* new_v = new RoadVertex(v->getPt());
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads2->graph);
		new_roads2->graph[new_v_desc] = new_v;

		convV2[*vi] = new_v_desc;
	}

	// roads1のエッジを、new_roads1へコピーする
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads1->graph);
		RoadEdge* e = roads1->graph[*ei];

		bool created = false;
		if (!roads1->sibling.contains(src) && !roads1->sibling.contains(tgt)) {
			// 両頂点とも、もともと複数の相手と対応していないので、そのままエッジを作ればよい
			RoadEdge* new_e = new RoadEdge(1, 1);
			for (int i = 0; i < e->getPolyLine().size(); i++) {
				new_e->addPoint(e->getPolyLine()[i]);
			}
			std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(convV1[src], convV1[tgt], new_roads1->graph);
			new_roads1->graph[new_e_pair.first] = new_e;

			created = true;
		} else if (!hasEdge(roads1, src, tgt)) {
			// roads1の該当頂点の兄弟の中で、既にエッジがあるかチェックする。
			if (!hasEdge(roads1, src, tgt)) {
				RoadEdge* new_e = new RoadEdge(1, 1);
				new_e->addPoint(roads1->graph[src]->getPt());
				new_e->addPoint(roads1->graph[tgt]->getPt());
				std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(convV1[src], convV1[tgt], new_roads1->graph);
				new_roads1->graph[new_e_pair.first] = new_e;

				created = true;
			}

		} else {
		}



		// エッジが作られた場合、
		if (created) {
			// 対応するグラフ２の頂点が兄弟なら、または、対応する頂点間にもともとエッジがあったなら、エッジを作成する。
			if (isSibling(roads2->sibling, neighbor1[src], neighbor1[tgt])
				|| hasEdge(roads2, neighbor1[src], neighbor1[tgt])) {
				RoadEdge* new_e = new RoadEdge(1, 1);
				new_e->addPoint(new_roads2->graph[convV2[neighbor1[src]]]->getPt());
				new_e->addPoint(new_roads2->graph[convV2[neighbor1[tgt]]]->getPt());

				std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(convV2[neighbor1[src]], convV2[neighbor1[tgt]], new_roads2->graph);
				new_roads2->graph[new_e_pair.first] = new_e;
			}
		}
	}


}

RoadGraph* Morph::copyGraphVertices(RoadGraph* roads) {
	RoadGraph* new_roads = new RoadGraph();

	QMap<RoadVertexDesc, RoadVertexDesc> convV;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		RoadVertex* v = roads->graph[*vi];

		RoadVertex* new_v = new RoadVertex(v->getPt());
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
		new_roads->graph[new_v_desc] = new_v;

		convV[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);
		RoadEdge* e = roads->graph[*ei];

		RoadEdge* new_e = new RoadEdge(1, 1);
		for (int i = 0; i < e->getPolyLine().size(); i++) {
			new_e->addPoint(e->getPolyLine()[i]);
		}

		std::pair<RoadEdgeDesc, bool> new_edge = boost::add_edge(convV[src], convV[tgt], new_roads->graph);
		roads->graph[new_edge.first] = new_e;
	}

	return new_roads;
}

bool Morph::isSibling(QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > sibling, RoadVertexDesc desc1, RoadVertexDesc desc2) {
	for (int i = 0; i < sibling[desc1].size(); i++) {
		if (sibling[desc1][i] == desc2) return true;
	}

	return false;
}

/**
 * ２つの頂点間にエッジがあるかチェックする。
 * 各頂点の兄弟も含めてチェックする。つまり、頂点Ａの兄弟Ａ’と、頂点Ｂの兄弟Ｂ’の間にエッジがあれば、
 * trueを返却する。
 */
bool Morph::hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2) {
	for (int i = 0; i < roads->sibling[desc1].size(); i++) {
		for (int j = 0; j < roads->sibling[desc2].size(); j++) {
			if (hasExclusiveEdge(roads1, roads->sibling[desc1][i], roads->sibling[desc2][j])) return true;
		}
	}

	return false;
}

/**
 * ２つの頂点間にエッジがあるかチェックする。
 */
bool Morph::hasExclusiveEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		if (src == desc1 && tgt == desc2) return true;
		if (tgt == desc1 && src == desc2) return true;
	}

	return false;
}