#include "RoadGraph.h"
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

	sibling.clear();

	graph.clear();
}

