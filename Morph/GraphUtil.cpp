#include "GraphUtil.h"
#include "Util.h"

#ifndef M_PI
#define M_PI	3.141592653
#endif

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

/**
 * 頂点の数を返却する。
 * onlyValidVertexフラグがtrueの場合は、全ての頂点のvalidフラグをチェックしてカウントする。
 *
 */
int GraphUtil::getNumVertices(RoadGraph* roads, bool onlyValidVertex) {
	if (!onlyValidVertex) {
		return boost::num_vertices(roads->graph);
	}

	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		count++;
	}

	return count;
}

/**
 * index番目の頂点を返却する。
 * onlyValidVertexフラグがtrueの場合は、有効な頂点のみをカウントする。
 */
RoadVertexDesc GraphUtil::getVertex(RoadGraph* roads, int index, bool onlyValidVertex) {
	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (onlyValidVertex && !roads->graph[*vi]->valid) continue;

		if (count == index) return *vi;

		count++;
	}

	throw "Index exceeds the number of vertices.";
}

/**
 * 当該頂点が、何番目の頂点かを返却する。
 */
int GraphUtil::getVertexIndex(RoadGraph* roads, RoadVertexDesc desc, bool onlyValidVertex) {
	int count = 0;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (onlyValidVertex && !roads->graph[*vi]->valid) continue;

		if (*vi == desc) return count;

		count++;
	}

	throw "The specified vertex does not exist.";
}

/**
 * グラフのノードv1をv2にcollapseする。
 */
void GraphUtil::collapseVertex(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2) {
	if (v1 == v2) return;

	roads->graph[v1]->valid = false;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v1, roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		// v1の隣接ノードv1bを取得
		RoadVertexDesc v1b = boost::target(*ei, roads->graph);

		// v1 - v1b間のエッジを無効にする
		roads->graph[*ei]->valid = false;

		// v2 - v1b間が既に接続されているなら、エッジは作成しない
		if (isDirectlyConnected(roads, v2, v1b)) continue;

		// v2 - v1b間にエッジを作成する
		RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
		new_e->addPoint(roads->graph[v2]->getPt());
		new_e->addPoint(roads->graph[v1b]->getPt());

		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(v2, v1b, roads->graph);
		roads->graph[edge_pair.first] = new_e;
	}
}

/**
 * グラフの指定エッジを削除する。Degreeの小さいほうの頂点を削除する。
 * また、木構造のための情報も併せて登録する。
 */
void GraphUtil::collapseEdge(RoadGraph* roads, RoadEdgeDesc e) {
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

/**
 * 頂点vの隣接ノードのリストを返却する
 */
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
 * ２つの頂点が直接接続されているか、チェックする。
 * ２つの頂点が同じ頂点の場合も、trueを返却する。
 */
bool GraphUtil::isDirectlyConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge) {
	if (desc1 == desc2) return true;
	return hasEdge(roads, desc1, desc2, onlyValidEdge);
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
 * 指定したエッジについて、polyLineのリストを、src頂点から並ぶようにして返却する。
 */
std::vector<QVector2D> GraphUtil::getOrderedPolyLine(RoadGraph* roads, RoadEdgeDesc e) {
	std::vector<QVector2D> ret = roads->graph[e]->getPolyLine();

	RoadVertexDesc src = boost::source(e, roads->graph);
	RoadVertexDesc tgt = boost::target(e, roads->graph);
	if ((roads->graph[src]->getPt() - roads->graph[e]->getPolyLine()[0]).length() < (roads->graph[tgt]->getPt() - roads->graph[e]->getPolyLine()[0]).length()) {
		return ret;
	} else {
		std::reverse(ret.begin(), ret.end());
		return ret;
	}
}

/**
 * 指定した頂点のDegreeを返却する。
 * onlyValidEdge = trueの場合は、validのエッジのみをカウントする。
 */
int GraphUtil::getDegree(RoadGraph* roads, RoadVertexDesc v, bool onlyValidEdge) {
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
		if (!roads->graph[*vi]->valid) continue;

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
		if (!roads->graph[*vi]->valid) continue;

		float dist = (roads->graph[*vi]->getPt() - pt).length();
		if (dist < min_dist) {
			nearest_desc = *vi;
			min_dist = dist;
		}
	}

	return nearest_desc;
}

/**
 * 指定したノードvと接続されたノードの中で、指定した座標に最も近いノードを返却する。
 */
RoadVertexDesc GraphUtil::findConnectedNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc v) {
	QMap<RoadVertexDesc, bool> visited;
	std::list<RoadVertexDesc> seeds;
	seeds.push_back(v);

	float min_dist = std::numeric_limits<float>::max();
	RoadVertexDesc min_desc;

	while (!seeds.empty()) {
		RoadVertexDesc seed = seeds.front();
		seeds.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(seed, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			RoadVertexDesc v2 = boost::target(*ei, roads->graph);
			if (visited.contains(v2)) continue;

			// 指定したノードvは除く（除かない方が良いのか？検討中。。。。）
			//if (v2 == v) continue;

			visited[v2] = true;

			// 指定した座標との距離をチェック
			float dist = (roads->graph[v2]->getPt() - pt).length();
			if (dist < min_dist) {
				min_dist = dist;
				min_desc = v2;
			}

			seeds.push_back(v2);
		}
	}

	return min_desc;
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

/**
 * ノード間の距離が指定した距離よりも近い場合は、１つにしてしまう。
 * また、２つのエッジのなす角度が小さすぎる場合は、短いエッジを削除する。
 */
void GraphUtil::simplify(RoadGraph* roads, float dist_threshold, float angle_threshold) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		while (true) {
			RoadVertexDesc v2 = findNearestNeighbor(roads, roads->graph[*vi]->getPt(), *vi);
			if ((roads->graph[v2]->getPt() - roads->graph[*vi]->getPt()).length() > dist_threshold) break;

			collapseVertex(roads, v2, *vi);
		}

		/*
		// エッジ間のなす角が小さすぎる場合は、短いエッジを削除
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			std::vector<QVector2D> polyLine = getOrderedPolyLine(roads, *ei);
			float angle = atan2f((polyLine[1] - polyLine[0]).y(), (polyLine[1] - polyLine[0]).x());

			RoadOutEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = boost::out_edges(*vi, roads->graph); ei2 != eend2; ++ei2) {
				if (*ei == *ei2) continue;
				if (!roads->graph[*ei2]->valid) continue;

				std::vector<QVector2D> polyLine2 = getOrderedPolyLine(roads, *ei2);

				float angle2 = atan2f((polyLine2[1] - polyLine2[0]).y(), (polyLine2[1] - polyLine2[0]).x());
				if (fabs(angle - angle2) < angle_threshold) {
					if (roads->graph[*ei]->getLength() > roads->graph[*ei2]->getLength()) {
						roads->graph[*ei2]->valid = false;
					} else {
						roads->graph[*ei]->valid = false;
						break;
					}
				}
			}
		}
		*/
	}
}

/**
 * エッジのポリゴンが3つ以上で構成されている場合、中間点を全てノードとして登録する。
 */
void GraphUtil::normalize(RoadGraph* roads) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;
		if (roads->graph[*ei]->getPolyLine().size() <= 2) continue;

		// 当該エッジを無効にする
		roads->graph[*ei]->valid = false;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		RoadVertexDesc prev_desc;
		RoadVertexDesc last_desc;
		if ((roads->graph[src]->getPt() - roads->graph[*ei]->getPolyLine()[0]).length() < (roads->graph[tgt]->getPt() - roads->graph[*ei]->getPolyLine()[0]).length()) {
			prev_desc = src;
			last_desc = tgt;
		} else {
			prev_desc = tgt;
			last_desc = src;
		}

		for (int i = 1; i < roads->graph[*ei]->getPolyLine().size() - 1; i++) {
			// 中間点をノードとして登録
			RoadVertex* new_v = new RoadVertex(roads->graph[*ei]->getPolyLine()[i]);
			RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
			roads->graph[new_v_desc] = new_v;

			// エッジを作成
			RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
			new_e->addPoint(roads->graph[prev_desc]->getPt());
			new_e->addPoint(roads->graph[new_v_desc]->getPt());
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(prev_desc, new_v_desc, roads->graph);
			roads->graph[edge_pair.first] = new_e;

			prev_desc = new_v_desc;
		}

		// 最後のエッジを作成
		RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
		new_e->addPoint(roads->graph[prev_desc]->getPt());
		new_e->addPoint(roads->graph[last_desc]->getPt());
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(prev_desc, last_desc, roads->graph);
		roads->graph[edge_pair.first] = new_e;
	}
}

/**
 * ０番と接続されているノードのみ有効とし、それ以外のノード、およびエッジを、全て無効にする。
 * 本実装では、事前の有効・無効フラグを考慮していない。要検討。。。
 */
void GraphUtil::singlify(RoadGraph* roads) {
	// 一旦、すべてを無効にする
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		roads->graph[*vi]->valid = false;
	}
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		roads->graph[*ei]->valid = false;
	}

	std::list<RoadVertexDesc> seeds;
	seeds.push_back(0);
	QMap<RoadVertexDesc, bool> visited;

	while (!seeds.empty()) {
		RoadVertexDesc seed = seeds.front();
		seeds.pop_front();

		RoadOutEdgeIter oei, oeend;
		for (boost::tie(oei, oeend) = boost::out_edges(seed, roads->graph); oei != oeend; ++oei) {
			RoadVertexDesc v = boost::target(*oei, roads->graph);

			// 到達したノード、エッジを有効にする
			roads->graph[v]->valid = true;
			roads->graph[*oei]->valid = true;

			if (visited.contains(v)) continue;

			visited[v] = true;
			seeds.push_back(v);
		}
	}
}

/**
 * 道路網をplanarグラフにする。
 */
void GraphUtil::planarify(RoadGraph* roads) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadEdge* e = roads->graph[*ei];
		if (!e->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		RoadEdgeIter ei2, eend2;
		for (boost::tie(ei2, eend2) = boost::edges(roads->graph); ei2 != eend2; ++ei2) {
			RoadEdge* e2 = roads->graph[*ei2];
			if (!e2->valid) continue;

			RoadVertexDesc src2 = boost::source(*ei2, roads->graph);
			RoadVertexDesc tgt2 = boost::target(*ei2, roads->graph);

			//if ((src == src2 && tgt == tgt2) || (src == tgt2 && tgt == src2)) continue;
			if (src == src2 || src == tgt2 || tgt == src2 || tgt == tgt2) continue;

			for (int i = 0; i < e->getPolyLine().size() - 1; i++) {
				for (int j = 0; j < e2->getPolyLine().size() - 1; j++) {
					float tab, tcd;
					QVector2D intPoint;
					if (Util::segmentSegmentIntersectXY(e->getPolyLine()[i], e->getPolyLine()[i+1], e2->getPolyLine()[j], e2->getPolyLine()[j+1], &tab, &tcd, true, intPoint)) {
						// 交点をノードとして登録
						RoadVertex* new_v = new RoadVertex(intPoint);
						RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
						roads->graph[new_v_desc] = new_v;

						// もともとのエッジを無効にする
						roads->graph[*ei]->valid = false;
						roads->graph[*ei2]->valid = false;

						// 新たなエッジを追加する
						RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
						new_e->addPoint(roads->graph[src]->getPt());
						new_e->addPoint(roads->graph[new_v_desc]->getPt());
						std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(src, new_v_desc, roads->graph);
						roads->graph[edge_pair.first] = new_e;

						new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
						new_e->addPoint(roads->graph[new_v_desc]->getPt());
						new_e->addPoint(roads->graph[tgt]->getPt());
						edge_pair = boost::add_edge(new_v_desc, tgt, roads->graph);
						roads->graph[edge_pair.first] = new_e;

						new_e = new RoadEdge(roads->graph[*ei2]->lanes, roads->graph[*ei2]->type);
						new_e->addPoint(roads->graph[src2]->getPt());
						new_e->addPoint(roads->graph[new_v_desc]->getPt());
						edge_pair = boost::add_edge(src2, new_v_desc, roads->graph);
						roads->graph[edge_pair.first] = new_e;

						new_e = new RoadEdge(roads->graph[*ei2]->lanes, roads->graph[*ei2]->type);
						new_e->addPoint(roads->graph[new_v_desc]->getPt());
						new_e->addPoint(roads->graph[tgt2]->getPt());
						edge_pair = boost::add_edge(new_v_desc, tgt2, roads->graph);
						roads->graph[edge_pair.first] = new_e;
					}
				}
			}
		}
	}
}

/**
 * 道路網をコピー(deep copy)する。
 */
RoadGraph* GraphUtil::copyRoads(RoadGraph* roads) {
	RoadGraph* new_roads = new RoadGraph();
	
	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		// 道路の追加
		RoadVertex* new_v = new RoadVertex(roads->graph[*vi]->getPt());
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
		new_roads->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		RoadVertexDesc new_src = conv[src];
		RoadVertexDesc new_tgt = conv[tgt];

		// エッジの追加
		RoadEdge* new_e = new RoadEdge(roads->graph[*ei]->lanes, roads->graph[*ei]->type);
		new_e->addPoint(new_roads->graph[new_src]->getPt());
		new_e->addPoint(new_roads->graph[new_tgt]->getPt());
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(new_src, new_tgt, new_roads->graph);
		new_roads->graph[edge_pair.first] = new_e;
	}

	return new_roads;
}

float GraphUtil::diffAngle(QVector2D& dir1, QVector2D& dir2) {
	float ang1 = atan2f(dir1.y(), dir1.x());
	if (ang1 < 0) ang1 += M_PI * 2.0f;

	float ang2 = atan2f(dir2.y(), dir2.x());
	if (ang2 < 0) ang2 += M_PI * 2.0f;

	float diff = fabs(ang1 - ang2);
	if (diff > M_PI) diff = M_PI * 2.0f - diff;

	return diff;
}