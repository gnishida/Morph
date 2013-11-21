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
 * 指定された頂点をコピーする。
 */
RoadVertexDesc GraphUtil::copyVertex(RoadGraph* roads, RoadVertexDesc v, bool virtFlag) {
	RoadVertex* new_v = new RoadVertex(roads->graph[v]->getPt());
	new_v->virt = virtFlag;
	RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
	roads->graph[new_v_desc] = new_v;

	return new_v_desc;
}

/**
 * 指定された頂点を移動する
 * とりあえず、無効なエッジも含めて、隣接エッジを併せて移動する。
 */
void GraphUtil::moveVertex(RoadGraph* roads, RoadVertexDesc v, QVector2D pt) {
	// 隣接エッジを移動する
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		std::vector<QVector2D> polyLine = roads->graph[*ei]->getPolyLine();
		if ((polyLine[0] - roads->graph[v]->getPt()).lengthSquared() < (polyLine[0] - roads->graph[tgt]->getPt()).lengthSquared()) {
			std::reverse(polyLine.begin(), polyLine.end());
		}

		int num = polyLine.size();
		QVector2D dir = pt - roads->graph[v]->getPt();
		for (int i = 0; i < num - 1; i++) {
			polyLine[i] += dir * (float)i / (float)(num - 1);
		}
		polyLine[num - 1] = pt;

		roads->graph[*ei]->polyLine = polyLine;
	}

	// 指定された頂点を移動する
	roads->graph[v]->pt = pt;
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
		addEdge(roads, v2, v1b, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);
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

			RoadEdgeDesc new_e_desc = addEdge(roads, v1, v3, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);

			// 木構造のための情報を登録する
			addedEdges.push_back(new_e_desc);
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
 * エッジを追加する
 */
RoadEdgeDesc GraphUtil::addEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, unsigned int lanes, unsigned int type, bool oneWay) {
	RoadEdge* e = new RoadEdge(lanes, type, oneWay);
	e->addPoint(roads->graph[src]->getPt());
	e->addPoint(roads->graph[tgt]->getPt());

	std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(src, tgt, roads->graph);
	roads->graph[edge_pair.first] = e;

	return edge_pair.first;
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
RoadVertexDesc GraphUtil::findNearestVertex(RoadGraph* roads, const QVector2D &pt) {
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
RoadVertexDesc GraphUtil::findNearestVertex(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc ignore) {
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
 * 指定された点に最も近いエッジを返却する。
 */
RoadEdgeDesc GraphUtil::findNearestEdge(RoadGraph* roads, const QVector2D &pt, float& dist, QVector2D& closestPt, bool onlyValidEdge) {
	dist = std::numeric_limits<float>::max();
	RoadEdgeDesc min_e;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		RoadVertex* src = roads->graph[boost::source(*ei, roads->graph)];
		RoadVertex* tgt = roads->graph[boost::target(*ei, roads->graph)];

		if (onlyValidEdge && !src->valid) continue;
		if (onlyValidEdge && !tgt->valid) continue;

		QVector2D pt2;
		float d = Util::pointSegmentDistanceXY(src->getPt(), tgt->getPt(), pt, pt2);
		if (d < dist) {
			dist = d;
			min_e = *ei;
			closestPt = pt2;
		}
	}

	return min_e;
}

/**
 * 指定された頂点に最も近いエッジを返却する。
 * ただし、指定された頂点に隣接するエッジは、対象外とする。
 */
RoadEdgeDesc GraphUtil::findNearestEdge(RoadGraph* roads, RoadVertexDesc v, float& dist, QVector2D &closestPt, bool onlyValidEdge) {
	dist = std::numeric_limits<float>::max();
	RoadEdgeDesc min_e;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);
		if (v == src || v == tgt) continue;

		if (onlyValidEdge && !roads->graph[src]->valid) continue;
		if (onlyValidEdge && !roads->graph[tgt]->valid) continue;

		QVector2D pt2;
		float d = Util::pointSegmentDistanceXY(roads->graph[src]->getPt(), roads->graph[tgt]->getPt(), roads->graph[v]->getPt(), pt2);
		if (d < dist) {
			dist = d;
			min_e = *ei;
			closestPt = pt2;
		}
	}

	return min_e;
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
 * ノードとエッジ間の距離が、閾値よりも小さい場合も、エッジ上にノードを移してしまう。
 */
void GraphUtil::simplify(RoadGraph* roads, float dist_threshold) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		while (true) {
			RoadVertexDesc v2 = findNearestVertex(roads, roads->graph[*vi]->getPt(), *vi);
			if ((roads->graph[v2]->getPt() - roads->graph[*vi]->getPt()).length() > dist_threshold) break;

			collapseVertex(roads, v2, *vi);
		}

		// 当該頂点に最も近いエッジを探す
		QVector2D closestPt;
		float dist;
		RoadEdgeDesc e = GraphUtil::findNearestEdge(roads, *vi, dist, closestPt);
		if (dist < dist_threshold) {
			// 当該頂点を、エッジ上の点に移動する
			GraphUtil::moveVertex(roads, *vi, closestPt);

			// 当該エッジの両端頂点を取得
			RoadVertexDesc src = boost::source(e, roads->graph);
			RoadVertexDesc tgt = boost::target(e, roads->graph);

			// 当該エッジを無効にする
			roads->graph[e]->valid = false;

			// エッジの付け替え
			if (!GraphUtil::hasEdge(roads, src, *vi)) {
				addEdge(roads, src, *vi, roads->graph[e]->lanes, roads->graph[e]->type, roads->graph[e]->oneWay);
			}
			if (!GraphUtil::hasEdge(roads, tgt, *vi)) {
				addEdge(roads, tgt, *vi, roads->graph[e]->lanes, roads->graph[e]->type, roads->graph[e]->oneWay);
			}
		}
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
			addEdge(roads, prev_desc, new_v_desc, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);

			prev_desc = new_v_desc;
		}

		// 最後のエッジを作成
		addEdge(roads, prev_desc, last_desc, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);
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
						addEdge(roads, src, new_v_desc, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);

						addEdge(roads, new_v_desc, tgt, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);

						addEdge(roads, src2, new_v_desc, roads->graph[*ei2]->lanes, roads->graph[*ei2]->type, roads->graph[*ei2]->oneWay);

						addEdge(roads, new_v_desc, tgt2, roads->graph[*ei2]->lanes, roads->graph[*ei2]->type, roads->graph[*ei2]->oneWay);
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
		addEdge(new_roads, new_src, new_tgt, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);
	}

	return new_roads;
}

/**
 * ２つのデータリストの差の最小値を返却する。
 * 各データは、１回のみ比較に使用できる。
 */
float GraphUtil::computeMinDiff(std::vector<float> *data1, std::vector<float> *data2) {
	float ret = 0.0f;

	if (data1->size() <= data2->size()) {
		std::vector<bool> paired2;
		for (int i = 0; i < data2->size(); i++) {
			paired2.push_back(false);
		}

		for (int i = 0; i < data1->size(); i++) {
			float min_diff = std::numeric_limits<float>::max();


			int min_id = -1;
			for (int j = 0; j < data2->size(); j++) {
				if (paired2[j]) continue;

				float diff = diffAngle(data1->at(i), data2->at(j));
				if (diff < min_diff) {
					min_diff = diff;
					min_id = j;
				}
			}

			paired2[min_id] = true;
			ret += min_diff;
		}
	} else {
		std::vector<bool> paired1;
		for (int i = 0; i < data1->size(); i++) {
			paired1.push_back(false);
		}

		for (int i = 0; i < data2->size(); i++) {
			float min_diff = std::numeric_limits<float>::max();


			int min_id = -1;
			for (int j = 0; j < data1->size(); j++) {
				if (paired1[j]) continue;

				float diff = fabs(diffAngle(data2->at(i), data1->at(j)));
				if (diff < min_diff) {
					min_diff = diff;
					min_id = j;
				}
			}

			paired1[min_id] = true;
			ret += min_diff;
		}
	}

	return ret;
}

/**
 * 角度を正規化し、[-PI , PI]の範囲にする。
 */
float GraphUtil::normalizeAngle(float angle) {
	// まずは、正の値に変換する
	if (angle < 0.0f) {
		angle += ((int)(fabs(angle) / M_PI / 2.0f) + 1) * M_PI * 2;
	}

	// 次に、[0, PI * 2]の範囲にする
	angle -= (int)(angle / M_PI / 2.0f) * M_PI * 2;

	// 最後に、[-PI, PI]の範囲にする
	if (angle > M_PI) angle = M_PI * 2.0f - angle;

	return angle;
}

/**
 * 角度の差を、[0, PI]の範囲で返却する。
 */
float GraphUtil::diffAngle(QVector2D& dir1, QVector2D& dir2) {
	float ang1 = atan2f(dir1.y(), dir1.x());
	float ang2 = atan2f(dir2.y(), dir2.x());

	return fabs(normalizeAngle(ang1 - ang2));
}

/**
 * 角度の差を、[0, PI]の範囲で返却する。
 */
float GraphUtil::diffAngle(float angle1, float angle2) {
	return fabs(normalizeAngle(angle1 - angle2));
}

/**
 * ２つの道路網のトポロジーの違いの最小値を数値化して返却する。
 * トポロジーの違いなので、座標は一切関係ない。隣接ノードとの接続性のみを考慮する。
 */
float GraphUtil::computeMinDiffInTopology(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2) {
	int N = getNumVertices(roads1);
	int M = getNumVertices(roads2);

	std::vector<int> corr1;
	std::vector<int> corr2;
	for (int i = 0; i < N; i++) {
		corr1[i] = 0;
	}
	for (int i = 0; i < M; i++) {
		corr2[i] = 0;
	}

	float min_diff = std::numeric_limits<float>::max();
	std::vector<int> min_corr1;
	std::vector<int> min_corr2;

	/*
	while (true) {
		// corr1、corr2を元に、mapを生成する
		map1.clear();
		map2.clear();
		for (int i = 0; i < N; i++) {
			map1[i] = corr1[i];
			map2[map1[i]] = i;
		}

		// いろいろやって
		float diff = computeDiffInTopology(roads1, corr1, roads2, corr2);
		if (diff < min_diff) {
			min_corr1 = corr1;
			min_corr2 = corr2;
		}
		
		if (!nextSequence(corr1, M)) break;
	}
	*/

	// 最小値を出した時に、頂点変換テーブルを、返却する
	map1.clear();
	map2.clear();
	for (int i = 0; i < N; i++) {
		map1[i] = corr1[i];
	}
	for (int i = 0; i < M; i++) {
		map2[i] = corr2[i];
	}

	return min_diff;
}

/**
 * 対応する頂点が与えられている時に、２つの道路網のトポロジーの違いを数値化して返却する。
 * トポロジーの違いなので、座標は一切関係ない。隣接ノードとの接続性のみを考慮する。
 */
float GraphUtil::computeDiffInTopology(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> correspondence1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc> correspondence2) {
	float ret = 0.0f;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		RoadVertexDesc v2 = correspondence1[*vi];

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads1->graph); ei != eend; ++ei) {
			RoadVertexDesc v1b = boost::target(*ei, roads1->graph);
			RoadVertexDesc v2b = correspondence1[v1b];

			if (!hasEdge(roads2, v2, v2b)) {
				ret += roads1->graph[*ei]->getWeight();
			}
		}
	}

	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		RoadVertexDesc v1 = correspondence2[*vi];

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads2->graph); ei != eend; ++ei) {
			RoadVertexDesc v2b = boost::target(*ei, roads2->graph);
			RoadVertexDesc v1b = correspondence1[v2b];

			if (!hasEdge(roads1, v1, v1b)) {
				ret += roads2->graph[*ei]->getWeight();
			}
		}
	}

	return ret;
}

/**
 * 与えられた数列の、末尾の桁の値を１インクリメントする。
 * N進法なので、Nになったら、桁が繰り上がる。
 * ex. {1, 2, 3} => {2, 2, 3}
 * ex. {N-1, 3, 3} => {0, 4, 3}
 * ex. {N-1, N-1, 3} => {0, 0, 4}
 */
bool GraphUtil::nextSequence(std::vector<int>& seq, int N) {
	int index = 0;
	while (true) {
		if (seq[index] < N - 1) break;

		seq[index] = 0;
	}

	if (index < seq.size()) {
		seq[index]++;
		return true;
	} else {
		return false;
	}
}

/**
 * 道路の統計情報を出力する。
 * degreeのヒストグラムと、lane数のヒストグラムを標準出力に出力する。
 */
void GraphUtil::printStatistics(RoadGraph* roads) {
	int degreesHistogram[10];
	int lanesHistogram[10];

	for (int i = 0; i < 10; i++) {
		degreesHistogram[i] = 0;
		lanesHistogram[i] = 0;
	}

	// degreeのヒストグラムを作成
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		int degree = getDegree(roads, *vi);
		if (degree < 10) {
			degreesHistogram[degree]++;
		}
	}

	fprintf(stdout, "Degrees:\n");
	for (int i = 0; i < 10; i++) {
		fprintf(stdout, "%d: %d\n", i, degreesHistogram[i]);
	}

	// レーン数のヒストグラムを作成
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		int lanes = roads->graph[*ei]->lanes;
		if (lanes < 10) {
			lanesHistogram[lanes]++;
		}
	}

	fprintf(stdout, "Lanes:\n");
	for (int i = 0; i < 10; i++) {
		fprintf(stdout, "%d: %d\n", i, degreesHistogram[i]);
	}
}
