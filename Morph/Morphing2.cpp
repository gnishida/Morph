#include "Morphing2.h"
#include "GraphUtil.h"
#include <qdebug.h>
#include <time.h>

Morphing2::Morphing2() {
	roadsA = NULL;
	roadsB = NULL;
}

Morphing2::~Morphing2() {
}

void Morphing2::initRoads(const char* filename1, const char* filename2) {
	clock_t start, end;

	/*
	roadsA = buildGraph1();
	roadsB = buildGraph2();
	*/

	FILE* fp1 = fopen(filename1, "rb");
	FILE* fp2 = fopen(filename2, "rb");
	if (roadsA != NULL) {
		roadsA->clear();
		delete roadsA;
	}
	if (roadsB != NULL) {
		roadsB->clear();
		delete roadsB;
	}

	start = clock();

	roadsA = new RoadGraph();
	roadsA->load(fp1, 2);
	end = clock();
	qDebug() << "Roads A is loaded[ms]: " << 1000.0 * (double)(end - start) / (double)CLOCKS_PER_SEC;

	start = clock();

	roadsB = new RoadGraph();
	roadsB->load(fp2, 2);
	end = clock();
	qDebug() << "Roads B is loaded[ms]: " << 1000.0 * (double)(end - start) / (double)CLOCKS_PER_SEC;

	initCorrespondence(roadsA, &correspond1);
	initCorrespondence(roadsB, &correspond2);

	// 第１ステップ：各頂点について、直近の対応を探す
	start = clock();
	findBestPairs(roadsA, &correspond1, roadsB, &correspond2);
	end = clock();
	qDebug() << "Roads A found the nearest neighbor[ms]: " << 1000.0 * (double)(end - start) / (double)CLOCKS_PER_SEC;
	start = clock();
	//findBestPairs(roadsB, &correspond2, roadsA, &correspond1);
	end = clock();
	qDebug() << "Roads B found the nearest neighbor[ms]: " << 1000.0 * (double)(end - start) / (double)CLOCKS_PER_SEC;

	// 
}

RoadGraph* Morphing2::interpolate(float t) {
	RoadGraph* roads = new RoadGraph();

	QMap<RoadVertexDesc, QMap<RoadVertexDesc, RoadVertexDesc> > conv;

	// roadsAを元に、頂点を登録していく
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roadsA->graph); vi != vend; ++vi) {
		RoadVertex* v1 = roadsA->graph[*vi];
		if (!v1->valid) continue;

		QMap<RoadVertexDesc, RoadVertexDesc> new_v_descs;

		for (QSet<RoadVertexDesc>::iterator it = correspond1[*vi]->begin(); it != correspond1[*vi]->end(); ++it) {
			RoadVertex* v2 = roadsB->graph[*it];

			// interpolateされた座標に、頂点を登録する
			RoadVertex* new_v = new RoadVertex(v1->getPt() * t + v2->getPt() * (1 - t));
			RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
			roads->graph[new_v_desc] = new_v;

			new_v_descs[*it] = new_v_desc;
		}

		conv[*vi] = new_v_descs;
	}

	// roadsAを元に、エッジを登録していく
	for (boost::tie(vi, vend) = boost::vertices(roadsA->graph); vi != vend; ++vi) {
		RoadVertexDesc v1_desc = *vi;
		if (!roadsA->graph[v1_desc]->valid) continue;

		for (QSet<RoadVertexDesc>::iterator it = correspond1[*vi]->begin(); it != correspond1[*vi]->end(); ++it) {
			// v1と対応するノードv2を取得
			RoadVertexDesc v2_desc = *it;
			if (!roadsB->graph[v2_desc]->valid) continue;

			for (QSet<RoadVertexDesc>::iterator it2 = correspond1[*vi]->begin(); it2 != correspond1[*vi]->end(); ++it2) {
				// v1と対応するもう１つのノードv2bを取得
				RoadVertexDesc v2b_desc = *it2;
				if (!roadsB->graph[v2b_desc]->valid) continue;
				if (v2b_desc == v2_desc) continue;

				if (GraphUtil::isDirectlyConnected(roadsB, v2_desc, v2b_desc)) {
					// interpolateエッジを生成する
					RoadVertexDesc new_v1_desc = conv[v1_desc][v2_desc];
					RoadVertexDesc new_v2_desc = conv[v1_desc][v2b_desc];

					RoadEdge* new_e = new RoadEdge(1, 1);
					new_e->addPoint(roads->graph[new_v1_desc]->getPt());
					new_e->addPoint(roads->graph[new_v2_desc]->getPt());

					std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(new_v1_desc, new_v2_desc, roads->graph);
					roads->graph[new_e_pair.first] = new_e;
				}
			}

			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(v1_desc, roadsA->graph); oei != oeend; ++oei) {
				if (!roadsA->graph[*oei]->valid) continue;

				// v1と隣接するノードv1bを取得
				RoadVertexDesc v1b_desc = boost::target(*oei, roads->graph);
				if (!roadsA->graph[v1b_desc]->valid) continue;

				for (QSet<RoadVertexDesc>::iterator it2 = correspond1[v1b_desc]->begin(); it2 != correspond1[v1b_desc]->end(); ++it2) {
					// v1bと対応するノードv2bを取得
					RoadVertexDesc v2b_desc = *it2;
					if (!roadsB->graph[v2b_desc]->valid) continue;

					// v2 - v2b 間にエッジがあるかチェック
					if (GraphUtil::isDirectlyConnected(roadsB, v2_desc, v2b_desc)) {
						// interpolateエッジを生成する
						RoadVertexDesc new_v1_desc = conv[v1_desc][v2_desc];
						RoadVertexDesc new_v2_desc = conv[v1b_desc][v2b_desc];

						RoadEdge* new_e = new RoadEdge(1, 1);
						new_e->addPoint(roads->graph[new_v1_desc]->getPt());
						new_e->addPoint(roads->graph[new_v2_desc]->getPt());

						std::pair<RoadEdgeDesc, bool> new_e_pair = boost::add_edge(new_v1_desc, new_v2_desc, roads->graph);
						roads->graph[new_e_pair.first] = new_e;
					}
				}
			}
		}
	}

	return roads;
}

void Morphing2::initCorrespondence(RoadGraph* roads, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspondence) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		//QSet<RoadVertexDesc> set;
		correspondence->insert(*vi, new QSet<RoadVertexDesc>());
	}
}

/**
 * 第１ステップ：Roads1の各頂点について、roads2の中からベストのペアを探す。
 * 　１）ある閾値の範囲で、最も短い距離のペアを探し、それだけペアにする。
 * 　　　ペアが１つもできない場合は、相手グラフに頂点を追加して、強引にペアにする。
 *   ２）ペアができた頂点の隣接頂点について、その対応点を、１）の対応点の隣接頂点（１）の対応点も含めて）の中から選ぶ。
 * 　３）２）を繰り返す
 * 　４）この時点で、１）でペアになった頂点と繋がっていない頂点はとり残されている。
 * 　　　取り残された頂点について、再度、１）から繰り返す。
 * 　　　取り残された頂点がなくなるまで、繰り返す。
 */
void Morphing2::findBestPairs(RoadGraph* roads1, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond1, RoadGraph* roads2, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond2) {
	while (true) {
		// １）ある閾値の範囲で、最も短い距離のペアを探し、それだけペアにする。
		float min_dist = std::numeric_limits<float>::max();
		RoadVertexDesc min_v1_desc;
		RoadVertexDesc min_v2_desc;

		RoadVertexIter vi, vend;
		int count = 0;
		for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
			// 既にペアがあるなら、スキップ
			if (!correspond1->value(*vi)->empty()) continue;

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
		if (count == 0) break;;

		// ペアを作成
		correspond1->value(min_v1_desc)->insert(min_v2_desc);
		correspond2->value(min_v2_desc)->insert(min_v1_desc);

		propagatePairs(roads1, min_v1_desc, correspond1, roads2, min_v2_desc, correspond2);
	}
}

/**
 * ペアを広げていく。
 */
void Morphing2::propagatePairs(RoadGraph* roads1, RoadVertexDesc v1_desc_seed, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond1, RoadGraph* roads2, RoadVertexDesc v2_desc_seed, QMap<RoadVertexDesc, QSet<RoadVertexDesc>* >* correspond2) {
	QList<RoadVertexDesc> v1_desc_queue;
	QList<RoadVertexDesc> v2_desc_queue;

	v1_desc_queue.push_back(v1_desc_seed);
	v2_desc_queue.push_back(v2_desc_seed);

	while (!v1_desc_queue.empty()) {
		// シードとなる頂点v1、v2を取得
		RoadVertexDesc v1_desc = v1_desc_queue[0];
		v1_desc_queue.pop_front();
		RoadVertexDesc v2_desc = v2_desc_queue[0];
		v2_desc_queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v1_desc, roads1->graph); ei != eend; ++ei) {
			if (!roads1->graph[*ei]->valid) continue;

			// v1の隣接ノードv1bを取得
			RoadVertexDesc v1b_desc = boost::target(*ei, roads1->graph);
			if (!correspond1->value(v1b_desc)->empty()) continue;

			// v1bのペアとなるノードv2bを取得
			//RoadVertexDesc v2b_desc = GraphUtil::findConnectedNearestNeighbor(roads2, roads1->graph[v1b_desc]->getPt(), v2_desc);
			RoadVertexDesc v2b_desc = GraphUtil::findConnectedNearestNeighbor(roads2, roads1->graph[v1b_desc]->getPt() - roads1->graph[v1_desc]->getPt() + roads2->graph[v2_desc]->getPt(), v2_desc);

			// ペア情報を登録
			correspond1->value(v1b_desc)->insert(v2b_desc);
			correspond2->value(v2b_desc)->insert(v1b_desc);

			v1_desc_queue.push_back(v1b_desc);
			v2_desc_queue.push_back(v2b_desc);
		}

		for (boost::tie(ei, eend) = boost::out_edges(v2_desc, roads2->graph); ei != eend; ++ei) {
			if (!roads2->graph[*ei]->valid) continue;

			// v2の隣接ノードv2bを取得
			RoadVertexDesc v2b_desc = boost::target(*ei, roads2->graph);
			if (!correspond2->value(v2b_desc)->empty()) continue;

			// v2bのペアとなるノードv1bを取得
			RoadVertexDesc v1b_desc = GraphUtil::findConnectedNearestNeighbor(roads1, roads2->graph[v2b_desc]->getPt(), v1_desc);

			// ペア情報を登録
			correspond2->value(v2b_desc)->insert(v1b_desc);
			correspond1->value(v1b_desc)->insert(v2b_desc);

			v1_desc_queue.push_back(v1b_desc);
			v2_desc_queue.push_back(v2b_desc);
		}


		for (QSet<RoadVertexDesc>::iterator it = correspond1->value(11)->begin(); it != correspond1->value(11)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
		for (QSet<RoadVertexDesc>::iterator it = correspond1->value(2)->begin(); it != correspond1->value(2)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
		for (QSet<RoadVertexDesc>::iterator it = correspond1->value(40)->begin(); it != correspond1->value(40)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
		for (QSet<RoadVertexDesc>::iterator it = correspond2->value(40)->begin(); it != correspond2->value(40)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
		for (QSet<RoadVertexDesc>::iterator it = correspond2->value(41)->begin(); it != correspond2->value(41)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
		for (QSet<RoadVertexDesc>::iterator it = correspond2->value(39)->begin(); it != correspond2->value(39)->end(); ++it) {
			int a = *it;
			int k = 0;
		}
	}
}
