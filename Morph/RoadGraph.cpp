#include "RoadGraph.h"
#include "GraphUtil.h"
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

		if (edge->type & roadType) {
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(src, tgt, graph);
			graph[edge_pair.first] = edge;
		} else {
			delete edge;
		}
	}

	removeIsolatedVertices();
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

