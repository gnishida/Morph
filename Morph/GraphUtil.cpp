#include "GraphUtil.h"

float GraphUtil::getTotalEdgeLength(RoadGraph* roads, RoadVertexDesc v) {
	float ret = 0.0f;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		ret += roads->graph[*ei]->getLength();
	}

	return ret;
}

void GraphUtil::removeVertex(RoadGraph* roads, RoadVertexDesc v) {
	roads->graph[v]->valid = false;

	// to be implemented
}

void GraphUtil::removeEdge(RoadGraph* roads, RoadEdgeDesc e) {
	// 当該エッジを無効にする
	roads->graph[e]->valid = false;

	RoadVertexDesc v1 = boost::source(e, roads->graph);
	RoadVertexDesc v2 = boost::target(e, roads->graph);
	if (v1 == v2) return;

	// 頂点v1と連結している頂点リストを取得する
	std::vector<RoadVertexDesc> neighbors = getNeighbors(roads, v1);

	// 頂点v2を無効にする
	roads->graph[v2]->valid = false;

	// 頂点v2と連結している頂点を、頂点v1と接続する
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v2, roads->graph); ei != eend; ++ei) {
		RoadVertexDesc v3 = boost::target(*ei, roads->graph);
		if (v3 == v1) continue;

		if (isNeighbor(roads, v1, v3)) {
			roads->graph[*ei]->valid = false;
		} else {
			roads->graph[*ei]->valid = false;

			RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
			new_e->addPoint(roads->graph[v1]->getPt());
			new_e->addPoint(roads->graph[v3]->getPt());
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(v1, v3, roads->graph);
			roads->graph[edge_pair.first] = new_e;
		}
	}
}

std::vector<RoadVertexDesc> GraphUtil::getNeighbors(RoadGraph* roads, RoadVertexDesc v) {
	std::vector<RoadVertexDesc> neighbors;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		neighbors.push_back(boost::target(*ei, roads->graph));
	}

	return neighbors;
}

bool GraphUtil::isNeighbor(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2) {
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v1, roads->graph); ei != eend; ++ei) {
		if (boost::target(*ei, roads->graph) == v2) return true;
	}
	for (boost::tie(ei, eend) = boost::out_edges(v2, roads->graph); ei != eend; ++ei) {
		if (boost::target(*ei, roads->graph) == v1) return true;
	}

	return false;
}
