#include "GraphUtil.h"

/**
 * 指定した頂点から出ているエッジの長さの合計を返却する
 */
float GraphUtil::getTotalEdgeLength(RoadGraph* roads, RoadVertexDesc v) {
	float ret = 0.0f;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;
		ret += roads->graph[*ei]->getLength();
	}

	return ret;
}

void GraphUtil::removeVertex(RoadGraph* roads, RoadVertexDesc v) {
	roads->graph[v]->valid = false;

	// to be implemented
}

/**
 * グラフの指定エッジを削除する。Degreeの小さいほうの頂点を削除する。
 * また、木構造のための情報も併せて登録する。
 */
void GraphUtil::removeEdge(RoadGraph* roads, RoadEdgeDesc e) {
	// 当該エッジを無効にする
	roads->graph[e]->valid = false;

	RoadVertexDesc v1 = boost::source(e, roads->graph);
	RoadVertexDesc v2 = boost::target(e, roads->graph);
	if (v1 == v2) return;

	// degreeの多い方をv1とする
	if (boost::degree(v1, roads->graph) < boost::degree(v2, roads->graph)) {
		RoadVertexDesc temp = v1;
		v1 = v2;
		v2 = temp;
	}

	// 頂点v2を無効にする
	roads->graph[v2]->valid = false;

	// 木構造のための情報を登録する
	roads->childrenToParent[v2] = v1;
	CollapseAction ca;
	ca.parentNode = v1;
	ca.childNode = v2;

	std::vector<RoadEdgeDesc> removedEdges;
	std::vector<RoadEdgeDesc> addedEdges;

	// 頂点v2と連結している頂点を、頂点v1と接続する
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v2, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

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

			// 木構造のための情報を登録する
			addedEdges.push_back(edge_pair.first);
		}

		// 木構造のための情報を登録する
		removedEdges.push_back(*ei);
	}

	roads->childrenRemovedEdges[v2] = removedEdges;
	roads->parentAddedEdges[v2] = addedEdges;
	ca.addedEdges = addedEdges;
	ca.removedEdges = removedEdges;

	roads->collapseHistory.push_back(ca);
}

std::vector<RoadVertexDesc> GraphUtil::getNeighbors(RoadGraph* roads, RoadVertexDesc v) {
	std::vector<RoadVertexDesc> neighbors;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;
		neighbors.push_back(boost::target(*ei, roads->graph));
	}

	return neighbors;
}

bool GraphUtil::isNeighbor(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2) {
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v1, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;
		if (boost::target(*ei, roads->graph) == v2) return true;
	}
	for (boost::tie(ei, eend) = boost::out_edges(v2, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;
		if (boost::target(*ei, roads->graph) == v1) return true;
	}

	return false;
}

/**
 * srcからtgtに到達可能かどうかチェックする。
 */
bool GraphUtil::isReachable(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt) {
	std::vector<boost::default_color_type> color(boost::num_vertices(roads->graph), boost::white_color);
	return boost::is_reachable(src, tgt, roads->graph, color.data());
}

/**
 * ２つの頂点間にエッジがあるかチェックする。
 * ただし、無効フラグの立っているエッジは、エッジがないとみなす。
 */
bool GraphUtil::hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge) {
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(desc1, roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		RoadVertexDesc tgt = boost::target(*ei, roads->graph);
		if (tgt == desc2) return true;
	}

	for (boost::tie(ei, eend) = boost::out_edges(desc2, roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		RoadVertexDesc tgt = boost::target(*ei, roads->graph);
		if (tgt == desc1) return true;
	}

	return false;
}

/**
 * srcとtgtの間のエッジを返却する。
 */
RoadEdgeDesc GraphUtil::getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, bool onlyValidEdge) {
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(src, roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		if (boost::target(*ei, roads->graph) == tgt) return *ei;
	}

	for (boost::tie(ei, eend) = boost::out_edges(tgt, roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		if (boost::target(*ei, roads->graph) == src) return *ei;
	}

	throw "No edge found.";
}

/**
 * 指定した頂点のDegreeを返却する。
 * onlyValidEdge = trueの場合は、validのエッジのみをカウントする。
 */
int GraphUtil::getDegree(RoadGraph* roads, RoadVertexDesc v, bool onlyValidEdge = true) {
	if (onlyValidEdge) {
		int count = 0;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
			if (roads->graph[*ei]->valid) count++;
		}
		return count;
	} else {
		return boost::degree(v, roads->graph);
	}
}

/**
 * 対象グラフの中から、指定した点に最も近い頂点descを返却する。
 */
RoadVertexDesc GraphUtil::findNearestNeighbor(RoadGraph* roads, const QVector2D &pt) {
	RoadVertexDesc nearest_desc;
	float min_dist = std::numeric_limits<float>::max();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		float dist = (roads->graph[*vi]->getPt() - pt).length();
		if (dist < min_dist) {
			nearest_desc = *vi;
			min_dist = dist;
		}
	}

	return nearest_desc;
}

/**
 * 対象グラフの中から、指定した点に最も近い頂点descを返却する。
 * ただし、ignore頂点は、検索対象から外す。
 */
RoadVertexDesc GraphUtil::findNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc ignore) {
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
 * 親子関係の木構造に基づいて、指定した頂点の子ノードリストを返却する。
 */
std::vector<RoadVertexDesc> GraphUtil::getChildren(RoadGraph* roads, RoadVertexDesc v) {
	std::vector<RoadVertexDesc> ret;

	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = roads->childrenToParent.begin(); it != roads->childrenToParent.end(); ++it) {
		if (it.value() == v) {
			ret.push_back(it.key());
		}
	}

	return ret;
}