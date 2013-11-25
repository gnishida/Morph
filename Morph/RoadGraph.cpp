#include "RoadGraph.h"
#include "GraphUtil.h"
#include <qset.h>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#define SQR(x)		((x) * (x))

using namespace std;

RoadGraph::RoadGraph() {
}

RoadGraph::~RoadGraph() {
	clear();
}

void RoadGraph::clear() {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		RoadVertex* v = graph[*vi];
		delete v;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		RoadEdge* edge = graph[*ei];
		delete edge;
	}

	siblings.clear();

	graph.clear();
}

void RoadGraph::load(FILE* fp, int roadType) {
	clear();

	QMap<uint, RoadVertexDesc> idToDesc;

	// Read the number of vertices
	unsigned int nVertices;
	fread(&nVertices, sizeof(unsigned int), 1, fp);

	// Read each vertex's information: desc, x, and y.
	for (int i = 0; i < nVertices; i++) {
		RoadVertexDesc id;
		float x, y;
		fread(&id, sizeof(RoadVertexDesc), 1, fp);
		fread(&x, sizeof(float), 1, fp);
		fread(&y, sizeof(float), 1, fp);

		RoadVertex* vertex = new RoadVertex(QVector2D(x, y));

		RoadVertexDesc desc = boost::add_vertex(graph);
		graph[desc] = vertex;

		idToDesc[id] = desc;
	}

	// Read the number of edges
	unsigned int nEdges;
	fread(&nEdges, sizeof(unsigned int), 1, fp);

	// Read each edge's information: the descs of two vertices, road type, the number of lanes, the number of points along the polyline, and the coordinate of each point along the polyline.
	for (int i = 0; i < nEdges; i++) {
		RoadEdge* edge = new RoadEdge(1, 1, false);

		RoadVertexDesc id1, id2;
		fread(&id1, sizeof(RoadVertexDesc), 1, fp);
		fread(&id2, sizeof(RoadVertexDesc), 1, fp);

		RoadVertexDesc src = idToDesc[id1];
		RoadVertexDesc tgt = idToDesc[id2];

		fread(&edge->type, sizeof(unsigned int), 1, fp);
		fread(&edge->lanes, sizeof(unsigned int), 1, fp);
		fread(&edge->oneWay, sizeof(unsigned int), 1, fp);

		unsigned int nPoints;
		fread(&nPoints, sizeof(unsigned int), 1, fp);

		for (int j = 0; j < nPoints; j++) {
			float x, y;
			fread(&x, sizeof(float), 1, fp);
			fread(&y, sizeof(float), 1, fp);

			edge->addPoint(QVector2D(x, y));
		}

		// 指定されたタイプの道路エッジのみを読み込む
		if (((int)powf(2, (edge->type - 1)) & roadType)) {
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(src, tgt, graph);
			graph[edge_pair.first] = edge;
		} else {
			delete edge;
		}
	}

	removeIsolatedVertices();
}

void RoadGraph::save(FILE* fp) {
	int nVertices = GraphUtil::getNumVertices(this);
	fwrite(&nVertices, sizeof(int), 1, fp);

	// 各頂点につき、ID、X座標、Y座標を出力する
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		RoadVertex* v = graph[*vi];
		if (!v->valid) continue;
	
		RoadVertexDesc desc = *vi;
		float x = v->getPt().x();
		float y = v->getPt().y();
		fwrite(&desc, sizeof(RoadVertexDesc), 1, fp);
		fwrite(&x, sizeof(float), 1, fp);
		fwrite(&y, sizeof(float), 1, fp);
	}

	int nEdges = GraphUtil::getNumEdges(this);
	fwrite(&nEdges, sizeof(int), 1, fp);

	// 各エッジにつき、２つの頂点の各ID、道路タイプ、レーン数、一方通行か、ポリラインを構成するポイント数、各ポイントのX座標とY座標を出力する
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		RoadEdge* edge = graph[*ei];
		if (!edge->valid) continue;

		RoadVertexDesc src = boost::source(*ei, graph);
		RoadVertexDesc tgt = boost::target(*ei, graph);

		fwrite(&src, sizeof(RoadVertexDesc), 1, fp);
		fwrite(&tgt, sizeof(RoadVertexDesc), 1, fp);
		
		unsigned int type = edge->getType();
		fwrite(&type, sizeof(unsigned int), 1, fp);

		unsigned int lanes = edge->getNumLanes();
		fwrite(&lanes, sizeof(unsigned int), 1, fp);

		unsigned int oneWay;
		if (edge->oneWay) {
			oneWay = 1;
		} else {
			oneWay = 0;
		}
		fwrite(&oneWay, sizeof(unsigned int), 1, fp);

		int nPoints = edge->getPolyLine().size();
		fwrite(&nPoints, sizeof(int), 1, fp);

		for (int i = 0; i < edge->getPolyLine().size(); i++) {
			float x = edge->getPolyLine()[i].x();
			float y = edge->getPolyLine()[i].y();
			fwrite(&x, sizeof(float), 1, fp);
			fwrite(&y, sizeof(float), 1, fp);
		}
	}
}

/**
 * エッジがない頂点を削除する。
 */
void RoadGraph::removeIsolatedVertices() {
	RoadVertexIter vi, vend;
	bool deleted = false;

	// 独立している頂点を削除
	do {
		deleted = false;

		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			RoadVertex* v = graph[*vi];

			if (boost::degree(*vi, graph) == 0) {
				boost::remove_vertex(*vi, graph);
				deleted = true;
				break;
			}
		}
	} while (deleted);
}

/**
 * 接続性情報を計算する。
 * 注意：頂点の追加、エッジの追加などにより、この情報は正しくなくなる！
 */
void RoadGraph::computeConnectivities() {
	connectivities.clear();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		if (!graph[*vi]->valid) continue;

		QList<RoadVertexDesc> list;
		list.push_back(*vi);

		std::list<RoadVertexDesc> queue;

		queue.push_back(*vi);

		// 各頂点について、到達できる頂点を全て洗い出す
		while (!queue.empty()) {
			RoadVertexDesc v = queue.front();
			queue.pop_front();

			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(v, graph); ei != eend; ++ei) {
				if (!graph[*ei]->valid) continue;

				RoadVertexDesc u = boost::target(*ei, graph);
				if (!graph[u]->valid) continue;

				if (list.contains(u)) continue;

				list.push_back(u);
				queue.push_back(u);
			}
		}

		connectivities[*vi] = list;
	}
}

bool RoadGraph::isConnected(RoadVertexDesc desc1, RoadVertexDesc desc2) {
	if (!connectivities.contains(desc1)) return false;

	return connectivities[desc1].contains(desc2);
}

/**
 * エッジの重みを計算する。
 * DeadEndを繰り返し削除し、残ったやつの重みを1、削除されたやつの重みを0とする。
 */
void RoadGraph::computeEdgeWeights() {
	RoadGraph* temp = GraphUtil::copyRoads(this);

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		if (!graph[*vi]->valid) continue;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, graph);

			if (GraphUtil::hasEdge(temp, *vi, tgt)) {
				graph[*ei]->weight = 1;
			} else {
				graph[*ei]->weight = 0;				
			}
		}
	}
}

/**
 * num個のメジャーなエッジを返却する。
 * レーン数が多い道路を選択する。
 */
std::vector<RoadEdgeDesc> RoadGraph::getMajorEdges(RoadGraph* roads, int num) {
	std::vector<RoadEdgeDesc> data;
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		if (!graph[*ei]->valid) continue;

		data.push_back(*ei);
	}

	std::sort(data.begin(), data.end(), LessWeight(roads));

	std::vector<RoadEdgeDesc> ret;
	for (int i = 0; i < num; i++) {
		ret.push_back(data[i]);
	}

	return ret;
}

LessWeight::LessWeight(RoadGraph* roads) {
	this->roads = roads;
}

bool LessWeight::operator()(const RoadEdgeDesc& left, const RoadEdgeDesc& right) const {
	return roads->graph[left]->lanes < roads->graph[right]->lanes;
}