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
	findBestPairs(roadsB, &correspond2, roadsA, &correspond1);
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

		QMap<RoadVertexDesc, RoadVertexDesc> new_v_descs;

		for (QSet<RoadVertexDesc>::iterator it = correspond1[*vi].begin(); it != correspond1[*vi].end(); ++it) {
			RoadVertex* v2 = roadsB->graph[*it];

			// 頂点を登録する
			RoadVertex* new_v = new RoadVertex(v1->getPt() * t + v2->getPt() * (1 - t));
			RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
			roads->graph[new_v_desc] = new_v;

			new_v_descs[*it] = new_v_desc;
		}

		conv[*v1] = new_v_descs;
	}

	// roadsAを元に、エッジを登録していく
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roadsA->graph); ei != eend; ++ei) {
		if (!roadsA->graph[*ei]->valid) continue;

		RoadVertexDesc src_desc = boost::source(*ei, roadsA->graph);
		RoadVertexDesc tgt_desc = boost::target(*ei, roadsA->graph);

		// src頂点の対応点でループ
		for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = conv[src_desc].begin(); it != conv[src_desc].end(); ++it) {
			RoadVertexDesc new_src_desc = conv[src_desc][it.key];

			// tgt頂点の対応点でループ

		}


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

void Morphing2::initCorrespondence(RoadGraph* roads, QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >* correspondence) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		std::vector<RoadVertexDesc> list;
		correspondence->insert(*vi, list);
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
void Morphing2::findBestPairs(RoadGraph* roads1, QMap<RoadVertexDesc, QSet<RoadVertexDesc> >* correspond1, RoadGraph* roads2, QMap<RoadVertexDesc, QSet<RoadVertexDesc> >* correspond2) {
	while (true) {
		// １）ある閾値の範囲で、最も短い距離のペアを探し、それだけペアにする。
		float min_dist = std::numeric_limits<float>::max();
		RoadVertexDesc min_v1_desc;
		RoadVertexDesc min_v2_desc;

		RoadVertexIter vi, vend;
		int count = 0;
		for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
			// 既にペアがあるなら、スキップ
			if (!correspond1->value[*vi]->empty()) continue;

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
		correspond1->value[min_v1_desc]->insert(min_v2_desc);
		correspond2->value[min_v2_desc]->insert(min_v1_desc);

		//propagatePairs(roads1, min_v1_desc, neighbor1, roads2, min_v2_desc, neighbor2);
	}
}
