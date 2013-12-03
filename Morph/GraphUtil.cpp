#include "GraphUtil.h"
#include "Util.h"
#include "Array2D.h"
#include "BFSForest.h"
#include <qlist.h>
#include <qmatrix.h>
#include <qdebug.h>

#ifndef M_PI
#define M_PI	3.141592653
#endif

EdgePair::EdgePair(RoadEdgeDesc edge1, RoadEdgeDesc edge2) {
	this->edge1 = edge1;
	this->edge2 = edge2;
}

EdgePairComparison::EdgePairComparison(RoadGraph* roads1, RoadGraph* roads2) {
	this->roads1 = roads1;
	this->roads2 = roads2;
}

/**
 * 非類似度が小さいものが先頭に来るようソートする。
 * つまり、似ているエッジペアが先頭に来る。
 */
bool EdgePairComparison::operator()(const EdgePair& left, const EdgePair& right) const {
	float dissimilarity1 = GraphUtil::computeDissimilarityOfEdges(roads1, left.edge1, roads2, left.edge2);
	float dissimilarity2 = GraphUtil::computeDissimilarityOfEdges(roads1, right.edge1, roads2, right.edge2);

	return dissimilarity1 < dissimilarity2;
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
 * 指定した頂点から辿れる頂点の数を返却する。
 */
int GraphUtil::getNumConnectedVertices(RoadGraph* roads, RoadVertexDesc start, bool onlyValidVertex) {
	int count = 1;

	QList<RoadVertexDesc> queue;
	queue.push_back(start);

	QList<RoadVertexDesc> visited;
	visited.push_back(start);

	while (!queue.empty()) {
		RoadVertexDesc v = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
			if (onlyValidVertex && !roads->graph[*ei]->valid) continue;

			RoadVertexDesc u = boost::target(*ei, roads->graph);
			if (onlyValidVertex && !roads->graph[u]->valid) continue;

			if (visited.contains(u)) continue;

			visited.push_back(u);
			queue.push_back(u);
			count++;
		}
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
 * 指定した座標からthreshold以内の頂点を探し、あればtrue返却する。
 * 存在しない場合は、falseを返却する。
 */
bool GraphUtil::getVertex(RoadGraph* roads, QVector2D pos, float threshold, RoadVertexDesc& desc, bool onlyValidVertex) {
	float min_dist = std::numeric_limits<float>::max();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (onlyValidVertex && !roads->graph[*vi]->valid) continue;

		float dist = (roads->graph[*vi]->getPt() - pos).length();
		if (dist < min_dist) {
			min_dist = dist;
			desc = *vi;
		}
	}

	if (min_dist <= threshold) return true;
	else return false;
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
 * 頂点を追加する。
 */
RoadVertexDesc GraphUtil::addVertex(RoadGraph* roads, RoadVertex* v) {
	RoadVertex* new_v = new RoadVertex(*v);
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
 * 頂点リストを取得する
 */
std::vector<RoadVertexDesc> GraphUtil::getVertices(RoadGraph* roads, bool onlyValidVertex) {
	std::vector<RoadVertexDesc> ret;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (onlyValidVertex && !roads->graph[*vi]->valid) continue;

		ret.push_back(*vi);
	}

	return ret;
}

/**
 * 孤立した頂点を無効にする
 */
void GraphUtil::removeIsolatedVertices(RoadGraph* roads, bool onlyValidVertex) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		if (getDegree(roads, *vi, onlyValidVertex) == 0) {
			roads->graph[*vi]->valid = false;
		}
	}
}

/**
 * index番目のエッジを返却する。
 */
RoadEdgeDesc GraphUtil::getEdge(RoadGraph* roads, int index, bool onlyValidEdge) {
	int count = 0;
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]) continue;

		if (index == count) return *ei;
		count++;
	}

	throw "No edge found for the specified index.";
}

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
 * エッジの数を返却する。
 */
int GraphUtil::getNumEdges(RoadGraph* roads, bool onlyValidEdge) {
	int count = 0;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (roads->graph[*ei]->valid) count++;
	}

	return count;
}

/**
 * エッジを追加する
 * 注意：この関数では、polyLineには両端ノードしか登録されない。つまり、直線のエッジとなる。
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
 * エッジを追加する
 * 参照エッジref_edgeを使って、レーン数、タイプ、一方通行、polyLineなどを設定する。
 */
RoadEdgeDesc GraphUtil::addEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, RoadEdge* ref_edge) {
	RoadEdge* e = new RoadEdge(*ref_edge);
	e->valid = true;

	std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(src, tgt, roads->graph);
	roads->graph[edge_pair.first] = e;

	return edge_pair.first;
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
 * 指定したエッジについて、指定した頂点からpolyLineが並ぶように変更する。
 */
void GraphUtil::orderPolyLine(RoadGraph* roads, RoadEdgeDesc e, RoadVertexDesc src) {
	RoadVertexDesc tgt;

	RoadVertexDesc s = boost::source(e, roads->graph);
	RoadVertexDesc t = boost::target(e, roads->graph);

	if (s == src) {
		tgt = t;
	} else {
		tgt = s;
	}

	// 順序が逆なら、リバースする
	if ((roads->graph[src]->getPt() - roads->graph[e]->getPolyLine()[0]).length() > (roads->graph[tgt]->getPt() - roads->graph[e]->getPolyLine()[0]).length()) {
		std::reverse(roads->graph[e]->polyLine.begin(), roads->graph[e]->polyLine.end());
	}
}

/**
 * 指定したエッジを、指定した位置に移動する。
 */
void GraphUtil::moveEdge(RoadGraph* roads, RoadEdgeDesc e, QVector2D& src_pos, QVector2D& tgt_pos) {
	RoadVertexDesc src = boost::source(e, roads->graph);
	RoadVertexDesc tgt = boost::target(e, roads->graph);

	QVector2D src_diff = src_pos - roads->graph[src]->pt;
	QVector2D tgt_diff = tgt_pos - roads->graph[tgt]->pt;

	if ((roads->graph[e]->polyLine[0] - roads->graph[src]->pt).length() < (roads->graph[e]->polyLine[0] - roads->graph[tgt]->pt).length()) {
		int n = roads->graph[e]->polyLine.size();
		for (int i = 1; i < n - 1; i++) {
			roads->graph[e]->polyLine[i] += src_diff + (tgt_diff - src_diff) * (float)i / (float)(n - 1);
		}
		roads->graph[e]->polyLine[0] = src_pos;
		roads->graph[e]->polyLine[n - 1] = tgt_pos;
	} else {
		int n = roads->graph[e]->polyLine.size();
		for (int i = 1; i < n - 1; i++) {
			roads->graph[e]->polyLine[i] += tgt_diff + (src_diff - tgt_diff) * (float)i / (float)(n - 1);
		}
		roads->graph[e]->polyLine[0] = tgt_pos;
		roads->graph[e]->polyLine[n - 1] = src_pos;
	}
}

/**
 * 全ての行き止まりエッジを削除する。
 */
bool GraphUtil::removeDeadEnd(RoadGraph* roads) {
	bool removed = false;

	bool removedOne = true;
	while (removedOne) {
		removedOne = false;
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
			if (!roads->graph[*vi]->valid) continue;

			if (getDegree(roads, *vi) == 1) {
				// 全ての隣接エッジを無効にする（実際は、１個しかないはず）
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
					roads->graph[*ei]->valid = false;
				}

				// 当該頂点を無効にする
				roads->graph[*vi]->valid = false;

				removedOne = true;
				removed = true;
			}
		}
	}

	return removed;
}

/**
 * ２つのpolyLineをinterpolateする。
 * 本実装は、簡易方式かも知れない。
 * ノード数が同じ場合は、順番にinterpolateするだけ。
 * ノード数が異なる場合は、polyLineを10個にdiscrete分割し、対応する２つの点をinterpolateする。
 */
std::vector<QVector2D> GraphUtil::interpolateEdges(RoadGraph* roads1, RoadEdgeDesc e1, RoadVertexDesc src1, RoadGraph* roads2, RoadEdgeDesc e2, RoadVertexDesc src2, float t) {

	orderPolyLine(roads1, e1, src1);
	orderPolyLine(roads2, e2, src2);

	std::vector<QVector2D> polyLine1 = roads1->graph[e1]->polyLine;
	std::vector<QVector2D> polyLine2 = roads2->graph[e2]->polyLine;

	std::vector<QVector2D> ret;

	int n1 = polyLine1.size();
	int n2 = polyLine2.size();

	if (n1 == n2) {
		for (int i = 0; i < n1; i++) {
			ret.push_back(polyLine1[i] * t + polyLine2[i] * (1.0f - t));
		}
	} else {
		for (float index = 0.0f; index < 1.0f; index += 0.1f) {
			// １つ目のpolyLineの、index番目の座標を計算
			int j1 = index * (float)(n1 - 1);

			float s1 = index - (float)j1 / (float)(n1 - 1);
			float t1 = (float)(j1 + 1) / (float)(n1 - 1) - index;

			QVector2D pt1 = polyLine1[j1] * t1 / (s1 + t1) + polyLine1[j1 + 1] * s1 / (s1 + t1);

			// ２つ目のpolyLineの、index番目の座標を計算
			int j2 = index * (float)(n2 - 1);

			float s2 = index - (float)j2 / (float)(n2 - 1);
			float t2 = (float)(j2 + 1) / (float)(n2 - 1) - index;

			QVector2D pt2 = polyLine2[j2] * t2 / (s2 + t2) + polyLine2[j2 + 1] * s2 / (s2 + t2);

			// interpolateする
			ret.push_back(pt1 * t + pt2 * (1.0f - t));
		}

		// 最後の点をinterpolateして追加する
		ret.push_back(polyLine1[polyLine1.size() - 1] * t + polyLine2[polyLine2.size() - 1] * (1.0f - t));
	}

	return ret;
}

/**
 * 道路網のエッジのImportanceを計算し、エッジのプロパティとして格納する。
 * 計算式：MAX( (w_length * length + w_valence * (valence1 + valence2) + w_lanes * lanes) / K, K / initial distance)
 */
void GraphUtil::computeImportanceOfEdges(RoadGraph* roads, float w_length, float w_valence, float w_lanes) {
	// エッジの最大長を計算する
	float max_length = 0.0f;
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		float length = roads->graph[*ei]->getLength();
		if (length > max_length) max_length = length;
	}
	
	float max_importance = 0.0f;
	int count = 0;

	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		roads->graph[*ei]->importance = roads->graph[*ei]->getLength() / max_length * w_length + (getDegree(roads, src) + getDegree(roads, tgt)) * w_valence + roads->graph[*ei]->lanes * w_lanes;
	}
}

/**
 * 道路網の２つのエッジの非類似性を計算して返却する。
 *
 * 計算式：２つの頂点の距離 + Angleの差 + Degreeの差 + レーン数の差
 */
float GraphUtil::computeDissimilarityOfEdges(RoadGraph* roads1, RoadEdgeDesc e1, RoadGraph* roads2, RoadEdgeDesc e2) {
	float w_distance = 0.001f;
	float w_angle = 1.25f;
	float w_degree = 0.3f;
	float w_lanes = 0.3f;

	RoadVertexDesc src1 = boost::source(e1, roads1->graph);
	RoadVertexDesc tgt1 = boost::target(e1, roads1->graph);

	RoadVertexDesc src2 = boost::source(e2, roads2->graph);
	RoadVertexDesc tgt2 = boost::target(e2, roads2->graph);

	// src1とtgt2、tgt1とsrc2が近い場合、src2、tgt2を入れ替える
	if ((roads1->graph[src1]->pt - roads2->graph[src2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[tgt2]->pt).length() > (roads1->graph[src1]->pt - roads2->graph[tgt2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[src2]->pt).length()) {
		src2 = boost::target(e2, roads2->graph);
		tgt2 = boost::source(e2, roads2->graph);
	}

	// 各要素を計算
	float dist = (roads1->graph[src1]->pt - roads2->graph[src2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[tgt2]->pt).length();
	float angle = diffAngle(roads1->graph[src1]->pt - roads1->graph[tgt1]->pt, roads2->graph[src2]->pt - roads2->graph[tgt2]->pt);
	float degree = abs(getDegree(roads1, src1) - getDegree(roads2, src2)) + abs(getDegree(roads1, tgt1) - getDegree(roads2, tgt2));
	float lanes = abs((double)(roads1->graph[e1]->lanes - roads2->graph[e2]->lanes));

	return dist * w_distance + angle * w_angle + degree * w_degree + lanes * w_lanes;
}

/**
 * 道路網をコピー(deep copy)する。
 * 無効な頂点、エッジもコピーし、無効フラグをつけておく。
 * こうすることで、頂点IDなどが、全く同じグラフが出来上がるはず。
 */
RoadGraph* GraphUtil::copyRoads(RoadGraph* roads) {
	RoadGraph* new_roads = new RoadGraph();
	
	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		// 道路の追加
		RoadVertex* new_v = new RoadVertex(roads->graph[*vi]->getPt());
		new_v->valid = roads->graph[*vi]->valid;
		RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
		new_roads->graph[new_v_desc] = new_v;	

		conv[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		RoadVertexDesc new_src = conv[src];
		RoadVertexDesc new_tgt = conv[tgt];

		// エッジの追加
		RoadEdge* new_e = new RoadEdge(*roads->graph[*ei]);
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(new_src, new_tgt, new_roads->graph);
		new_roads->graph[edge_pair.first] = new_e;
	}

	return new_roads;
}

/**
 * roads1をroads2にコピーする
 * 無効な頂点、エッジもコピーし、無効フラグをつけておく。
 * こうすることで、頂点IDなどが、全く同じグラフが出来上がるはず。
 */
void GraphUtil::copyRoads(RoadGraph* roads1, RoadGraph* roads2) {
	roads2->clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		// 道路の追加
		RoadVertex* new_v = new RoadVertex(roads1->graph[*vi]->getPt());
		new_v->valid = roads1->graph[*vi]->valid;
		RoadVertexDesc new_v_desc = boost::add_vertex(roads2->graph);
		roads2->graph[new_v_desc] = new_v;

		conv[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads1->graph);

		RoadVertexDesc new_src = conv[src];
		RoadVertexDesc new_tgt = conv[tgt];

		// エッジの追加
		RoadEdge* new_e = new RoadEdge(*roads1->graph[*ei]);
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(new_src, new_tgt, roads2->graph);
		roads2->graph[edge_pair.first] = new_e;
	}
}

/**
 * 道路網のAxis Alignedバウンディングボックスを返却する。
 * バウンディングボックスは、XY軸にalignされたボックスを使う。
 */
BBox GraphUtil::getAABoundingBox(RoadGraph* roads) {
	BBox box;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		box.addPoint(roads->graph[*vi]->getPt());
	}

	return box;
}

/**
 * 道路網のバウンディングボックスを返却する。
 * 
 * XY軸にalignされていないボックスも含めて、面積が最小となるものを探す。
 * 実際には、道路網を-90度から+90度まで、5度ずつ回転させ、Axis Alignedバウンディングボックスを計算し、
 * 最も面積の小さいものを選択する。
 * 注意：道路網は、回転させた結果の道路網に更新される！
 */
BBox GraphUtil::getBoudingBox(RoadGraph* roads, float theta1, float theta2, float theta_step) {
	float min_area = std::numeric_limits<float>::max();
	float min_theta;
	BBox min_box;

	for (float theta = theta1; theta <= theta2; theta += theta_step) {
		RoadGraph* rotated_roads = copyRoads(roads);
		rotate(rotated_roads, theta);
		BBox box = getAABoundingBox(rotated_roads);
		if (box.dx() * box.dy() < min_area) {
			min_area = box.dx() * box.dy();
			min_theta = theta;
			min_box = box;
		}

		delete rotated_roads;
	}

	rotate(roads, min_theta);
	return min_box;
}

/**
 * 道路網から、メイン道路だけを抽出し、新たな道路網を作成する。（スケルトンのイメージ）
 * removeフラグがtrueなら、抽出したエッジは、元の道路網ではinvalidにする。
 */
RoadGraph* GraphUtil::extractMajorRoad(RoadGraph* roads, bool remove) {
	float max_length = 0.0f;
	QList<RoadEdgeDesc> max_path;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		QList<RoadEdgeDesc> path;
		float length = extractMajorRoad(roads, *ei, path);
		if (length > max_length) {
			max_length = length;
			max_path = path;
		}
	}

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// メイン道路１本の道路網を生成する
	RoadGraph* new_roads = new RoadGraph();
	for (QList<RoadEdgeDesc>::iterator it = max_path.begin(); it != max_path.end(); ++it) {
		RoadVertexDesc src = boost::source(*it, roads->graph);
		RoadVertexDesc tgt = boost::target(*it, roads->graph);

		// src頂点に対応する頂点を取得、または、作成する
		RoadVertexDesc new_src;
		if (conv.contains(src)) {
			new_src = conv[src];
		} else {
			RoadVertex* v = new RoadVertex(roads->graph[src]->getPt());
			new_src = boost::add_vertex(new_roads->graph);
			new_roads->graph[new_src] = v;
			conv[src] = new_src;
		}

		// tgt頂点に対応する頂点を取得、または、作成する
		RoadVertexDesc new_tgt;
		if (conv.contains(tgt)) {
			new_tgt = conv[tgt];
		} else {
			RoadVertex* v = new RoadVertex(roads->graph[tgt]->getPt());
			new_tgt = boost::add_vertex(new_roads->graph);
			new_roads->graph[new_tgt] = v;
			conv[tgt] = new_tgt;
		}

		// エッジを作成する
		RoadEdgeDesc e = getEdge(roads, src, tgt);
		addEdge(new_roads, new_src, new_tgt, roads->graph[e]->lanes, roads->graph[e]->type, roads->graph[e]->oneWay);

		if (remove) {
			// 元の道路網からエッジを削除する
			roads->graph[e]->valid = false;
		}
	}

	return new_roads;
}

/**
 * 道路網のあるエッジからスタートし、最もまっすぐな道を返却する。
 */
float GraphUtil::extractMajorRoad(RoadGraph* roads, RoadEdgeDesc root, QList<RoadEdgeDesc>& path) {
	path.clear();
	path.push_back(root);

	float length = roads->graph[root]->getLength();

	RoadVertexDesc root1 = boost::source(root, roads->graph);
	RoadVertexDesc root2 = boost::target(root, roads->graph);

	QList<RoadVertexDesc> visited;
	visited.push_back(root1);
	visited.push_back(root2);

	QList<RoadVertexDesc> queue;
	queue.push_back(root1);
	queue.push_back(root2);

	QList<float> angles;
	QVector2D dir = roads->graph[root1]->getPt() - roads->graph[root2]->getPt();
	angles.push_back(atan2f(dir.y(), dir.x()));
	angles.push_back(atan2f(-dir.y(), -dir.x()));

	while (!queue.empty()) {
		RoadVertexDesc v = queue.front();
		queue.pop_front();

		float angle = angles.front();
		angles.pop_front();

		float min_angle;
		float min_diff_angle = std::numeric_limits<float>::max();
		RoadVertexDesc min_u;
		RoadEdgeDesc min_e;
		float len;

		// 当該頂点の隣接頂点について
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			RoadVertexDesc u = boost::target(*ei, roads->graph);
			if (!roads->graph[u]->valid) continue;

			// 訪問済みの頂点はスキップ
			if (visited.contains(u)) continue;

			QVector2D dir2 = roads->graph[u]->getPt() - roads->graph[v]->getPt();
			float angle2 = atan2f(dir2.y(), dir2.x());
			float diff_angle = diffAngle(angle2, angle);
			if (diff_angle < min_diff_angle) {
				min_diff_angle = diff_angle;
				min_angle = angle2;
				min_u = u;
				min_e = *ei;
				len = roads->graph[*ei]->getLength();
			}
		}

		// 角度20度以内なら、まっすぐな道とみなす
		if (min_diff_angle < M_PI * 20.0f / 180.0f) {
			path.push_back(min_e);
			length += len;

			queue.push_back(min_u);
			angles.push_back(min_angle);
			visited.push_back(min_u);
		}
	}

	return length;
}

/**
 * 頂点vの隣接ノードのリストを返却する
 */
std::vector<RoadVertexDesc> GraphUtil::getNeighbors(RoadGraph* roads, RoadVertexDesc v, bool onlyValidVertex) {
	std::vector<RoadVertexDesc> neighbors;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
		if (onlyValidVertex && !roads->graph[*ei]->valid) continue;

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
/*
bool GraphUtil::isReachable(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt) {
	std::vector<boost::default_color_type> color(boost::num_vertices(roads->graph), boost::white_color);
	return boost::is_reachable(src, tgt, roads->graph, color.data());
}
*/

/**
 * ２つの頂点が直接接続されているか、チェックする。
 * ２つの頂点が同じ頂点の場合も、trueを返却する。
 */
bool GraphUtil::isDirectlyConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge) {
	if (desc1 == desc2) return true;
	return hasEdge(roads, desc1, desc2, onlyValidEdge);
}

/**
 * desc1とdesc2が、１個、または、複数個のエッジによって接続されているかチェックする。
 */
bool GraphUtil::isConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge) {
	QList<RoadVertexDesc> seeds;
	QSet<RoadVertexDesc> visited;

	seeds.push_back(desc1);
	visited.insert(desc1);

	while (!seeds.empty()) {
		RoadVertexDesc v = seeds.front();
		seeds.pop_front();

		visited.insert(v);

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads->graph); ei != eend; ++ei) {
			if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

			RoadVertexDesc u = boost::target(*ei, roads->graph);
			if (onlyValidEdge && !roads->graph[u]->valid) continue;

			if (u == desc2) return true;

			if (!visited.contains(u)) seeds.push_back(u);			
		}
	}

	return false;
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
 * ただし、指定された距離より通り場合は、falseを返却する。
 */
bool GraphUtil::getEdge(RoadGraph* roads, const QVector2D &pt, float threshold, RoadEdgeDesc& e, bool onlyValidEdge) {
	float min_dist = std::numeric_limits<float>::max();
	RoadEdgeDesc min_e;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (onlyValidEdge && !roads->graph[*ei]->valid) continue;

		RoadVertex* src = roads->graph[boost::source(*ei, roads->graph)];
		RoadVertex* tgt = roads->graph[boost::target(*ei, roads->graph)];

		if (onlyValidEdge && !src->valid) continue;
		if (onlyValidEdge && !tgt->valid) continue;

		QVector2D pt2;
		for (int i = 0; i < roads->graph[*ei]->polyLine.size() - 1; i++) {
			float dist = Util::pointSegmentDistanceXY(roads->graph[*ei]->polyLine[i], roads->graph[*ei]->polyLine[i + 1], pt, pt2);
			if (dist < min_dist) {
				min_dist = dist;
				e = *ei;
			}
		}
	}

	if (min_dist < threshold) return true;
	else return false;
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
 * 無効頂点やエッジを全て削除する。
 */
void GraphUtil::clean(RoadGraph* roads) {
	RoadGraph* temp = GraphUtil::copyRoads(roads);

	roads->clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(temp->graph); vi != vend; ++vi) {
		if (!temp->graph[*vi]->valid) continue;

		// 道路の追加
		RoadVertex* new_v = new RoadVertex(temp->graph[*vi]->getPt());
		RoadVertexDesc new_v_desc = boost::add_vertex(roads->graph);
		roads->graph[new_v_desc] = new_v;	

		conv[*vi] = new_v_desc;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(temp->graph); ei != eend; ++ei) {
		if (!temp->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, temp->graph);
		RoadVertexDesc tgt = boost::target(*ei, temp->graph);

		RoadVertexDesc new_src = conv[src];
		RoadVertexDesc new_tgt = conv[tgt];

		// エッジの追加
		RoadEdge* new_e = new RoadEdge(*temp->graph[*ei]);
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(new_src, new_tgt, roads->graph);
		roads->graph[edge_pair.first] = new_e;
	}
}

/**
 * degreeが２の頂点を削除し、エッジの一部にする。
 */
void GraphUtil::reduce(RoadGraph* roads) {
	// reduce the graph by removing the vertices which have two outing edges.
	RoadVertexIter vi, vend;
	bool deleted = false;
	do {
		deleted = false;

		for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
			if (!roads->graph[*vi]->valid) continue;

			RoadVertex* v = roads->graph[*vi];

			if (getDegree(roads, *vi) == 2) {
				if (reduce(roads, *vi)) {
					deleted = true;
					break;
				}
			}
		}
	} while (deleted);
}

/**
 * degreeが２の指定された頂点を削除し、エッジの一部にする。
 */
bool GraphUtil::reduce(RoadGraph* roads, RoadVertexDesc desc) {
	int count = 0;
	RoadVertexDesc vd[2];
	RoadEdgeDesc ed[2];
	RoadEdge* edges[2];

	RoadOutEdgeIter ei, ei_end;
	for (boost::tie(ei, ei_end) = out_edges(desc, roads->graph); ei != ei_end; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		vd[count] = boost::target(*ei, roads->graph);
		ed[count] = *ei;
		edges[count] = roads->graph[*ei];
		count++;
	}

	//if (edges[0]->getType() != edges[1]->getType()) return false;
	//if (edges[0]->lanes != edges[1]->lanes) return false;

	// 三角形の形をしている場合、degreeが2でもreduceしない
	if (hasEdge(roads, vd[0], vd[1])) return false;

	RoadEdge* new_edge = new RoadEdge(edges[0]->oneWay, edges[0]->lanes, edges[0]->type);
	orderPolyLine(roads, ed[0], vd[0]);
	orderPolyLine(roads, ed[1], desc);
	
	for (int i = 0; i < edges[0]->getPolyLine().size(); i++) {
		new_edge->addPoint(edges[0]->getPolyLine()[i]);
	}
	for (int i = 1; i < edges[1]->getPolyLine().size(); i++) {
		new_edge->addPoint(edges[1]->getPolyLine()[i]);
	}
	std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(vd[0], vd[1], roads->graph);
	roads->graph[edge_pair.first] = new_edge;

	// 古いエッジを無効にする
	roads->graph[ed[0]]->valid = false;
	roads->graph[ed[1]]->valid = false;

	// 当該頂点を無効にする
	roads->graph[desc]->valid = false;

	return true;
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
 * start頂点と接続されているノードのみ有効とし、それ以外のノード、およびエッジを、全て無効にする。
 * 本実装では、事前の有効・無効フラグを考慮していない。要検討。。。
 */
void GraphUtil::singlify(RoadGraph* roads) {
	int max_size = 0;
	RoadVertexDesc start;

	// 最も大きいかたまり（接続されている）の道路網を探し出す
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		int size = getNumConnectedVertices(roads, *vi);
		if (size > max_size) {
			max_size = size;
			start = *vi;
		}
	}

	RoadGraph* new_roads = new RoadGraph();

	std::list<RoadVertexDesc> queue;
	queue.push_back(start);

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// スタート頂点を追加
	RoadVertex* new_v = new RoadVertex(roads->graph[start]->getPt());
	RoadVertexDesc new_v_desc = boost::add_vertex(new_roads->graph);
	new_roads->graph[new_v_desc] = new_v;

	conv[start] = new_v_desc;

	std::list<RoadVertexDesc> queue2;
	queue2.push_back(new_v_desc);

	while (!queue.empty()) {
		RoadVertexDesc v_desc = queue.front();
		queue.pop_front();

		RoadVertexDesc new_v_desc = queue2.front();
		queue2.pop_front();


		RoadOutEdgeIter oei, oeend;
		for (boost::tie(oei, oeend) = boost::out_edges(v_desc, roads->graph); oei != oeend; ++oei) {
			if (!roads->graph[*oei]->valid) continue;

			RoadVertexDesc u_desc = boost::target(*oei, roads->graph);
			if (!roads->graph[u_desc]->valid) continue;

			RoadVertexDesc new_u_desc;

			if (conv.contains(u_desc)) {
				new_u_desc = conv[u_desc];
			} else {
				// 新しいノードを作成する。
				RoadVertex* new_u = new RoadVertex(roads->graph[u_desc]->getPt());
				new_u_desc = boost::add_vertex(new_roads->graph);
				new_roads->graph[new_u_desc] = new_u;
			}

			// エッジを作成する。
			if (!hasEdge(new_roads, new_v_desc, new_u_desc)) {
				addEdge(new_roads, new_v_desc, new_u_desc, roads->graph[*oei]);
			}

			if (!conv.contains(u_desc)) {
				conv[u_desc] = new_u_desc;
				queue.push_back(u_desc);
				queue2.push_back(new_u_desc);
			}
		}
	}

	// new_roads を roads にコピーする
	copyRoads(new_roads, roads);
	delete new_roads;
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
 * 道路網をスケルトン化する。
 * 具体的には、オリジナル道路網で、degreeが1の頂点と、その隣接エッジを無効にする。
 * 注意：頂点を削除した結果、新たにdegreeが1となる頂点は、その対象ではない。
 */
void GraphUtil::skeltonize(RoadGraph* roads) {
	QList<RoadVertexDesc> list;

	// 削除対象となる頂点リストを取得
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		if (getDegree(roads, *vi) == 1) {
			list.push_back(*vi);
		}
	}

	for (int i = 0; i < list.size(); i++) {
		// 隣接エッジを無効にする
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(list[i], roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			roads->graph[*ei]->valid = false;
		}

		// 頂点を無効にする
		roads->graph[list[i]]->valid = false;
	}
}

/**
 * 道路網をtheta [rad] 回転する
 *
 * 無効の頂点、エッジも、とりあえず回転しておく。
 */
void GraphUtil::rotate(RoadGraph* roads, float theta) {
	// 頂点を回転する
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		QVector2D pos = roads->graph[*vi]->pt;

		roads->graph[*vi]->pt.setX(cosf(theta) * pos.x() - sinf(theta) * pos.y());
		roads->graph[*vi]->pt.setY(sinf(theta) * pos.x() + cosf(theta) * pos.y());
	}

	// エッジを回転する
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		for (int i = 0; i < roads->graph[*ei]->polyLine.size(); i++) {
			QVector2D pos = roads->graph[*ei]->polyLine[i];
			roads->graph[*ei]->polyLine[i].setX(cosf(theta) * pos.x() - sinf(theta) * pos.y());
			roads->graph[*ei]->polyLine[i].setY(sinf(theta) * pos.x() + cosf(theta) * pos.y());
		}
	}
}

/**
 * 道路網を移動する
 *
 * 無効の頂点、エッジも、とりあえず移動しておく。
 */
void GraphUtil::translate(RoadGraph* roads, QVector2D offset) {
	// 頂点を移動する
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		roads->graph[*vi]->pt += offset;
	}

	// エッジを移動する
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		for (int i = 0; i < roads->graph[*ei]->polyLine.size(); i++) {
			roads->graph[*ei]->polyLine[i] += offset;
		}
	}
}

/**
 * 道路網をグリッド型に無理やり変換する。
 * 頂点startから開始し、エッジの方向に基づいて、上下左右方向に、ノードを広げていくイメージ。
 */
RoadGraph* GraphUtil::convertToGridNetwork(RoadGraph* roads, RoadVertexDesc start) {
 	RoadGraph* new_roads = new RoadGraph();

	QList<RoadVertexDesc> queue;
	queue.push_back(start);

	QList<RoadVertexDesc> visited;
	visited.push_back(start);

	// スタート頂点を追加
	RoadVertex* v = new RoadVertex(QVector2D(0, 0));
	RoadVertexDesc v_desc = boost::add_vertex(new_roads->graph);
	new_roads->graph[v_desc] = v;
	
	QList<RoadVertexDesc> new_queue;
	new_queue.push_back(v_desc);

	while (!queue.empty()) {
		RoadVertexDesc v_desc = queue.front();
		queue.pop_front();
		RoadVertexDesc new_v_desc = new_queue.front();
		new_queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			RoadVertexDesc u_desc = boost::target(*ei, roads->graph);
			if (!roads->graph[u_desc]->valid) continue;

			// オリジナルの道路網で、エッジの方向を取得
			QVector2D dir = roads->graph[u_desc]->getPt() - roads->graph[v_desc]->getPt();

			QVector2D pos;
			if (diffAngle(dir, QVector2D(1, 0)) < M_PI * 0.25f) { // X軸正方向
				pos = new_roads->graph[new_v_desc]->getPt() + QVector2D(100.0f, 0.0f);
			} else if (diffAngle(dir, QVector2D(0, 1)) < M_PI * 0.25f) { // Y軸正方向
				pos = new_roads->graph[new_v_desc]->getPt() + QVector2D(0.0f, 100.0f);
			} else if (diffAngle(dir, QVector2D(-1, 0)) < M_PI * 0.25f) { // X軸負方向
				pos = new_roads->graph[new_v_desc]->getPt() + QVector2D(-100.0f, 0.0f);
			} else if (diffAngle(dir, QVector2D(0, -1)) < M_PI * 0.25f) { // Y軸負方向
				pos = new_roads->graph[new_v_desc]->getPt() + QVector2D(0.0f, -100.0f);
			} 
			
			RoadVertexDesc new_u_desc;
			if (!getVertex(new_roads, pos, 0.0f, new_u_desc)) {
				// 頂点を追加
				RoadVertex* new_u = new RoadVertex(pos);
				new_u_desc = boost::add_vertex(new_roads->graph);
				new_roads->graph[new_u_desc] = new_u;
			}

			if (!hasEdge(new_roads, new_v_desc, new_u_desc)) {
				// エッジを追加
				addEdge(new_roads, new_v_desc, new_u_desc, roads->graph[*ei]->lanes, roads->graph[*ei]->type, roads->graph[*ei]->oneWay);
			}

			if (!visited.contains(u_desc)) {
				visited.push_back(u_desc);

				queue.push_back(u_desc);
				new_queue.push_back(new_u_desc);
			}
		}
	}

	return new_roads;
}

/**
 * 道路網をグリッド型のネットワークで近似する。
 * 11/23に実装。急ぎで実装し、ほとんどテストしていない。バグがある可能性大。
 * 
 * @param cellLength		１つのセルの１辺の長さ
 * @param orig				原点の座標
 */
RoadGraph* GraphUtil::approximateToGridNetwork(RoadGraph* roads, float cellLength, QVector2D orig) {
	Array2D<RoadVertexDesc> grid_desc;
	Array2D<float> grid_weight;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		int u = (roads->graph[*vi]->getPt().x() - orig.x()) / cellLength;
		int v = (roads->graph[*vi]->getPt().y() - orig.y()) / cellLength;

		if (u < 0 || v < 0) continue;

		// 当該頂点の重要度を計算する
		float weight = 0.0f;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			weight += roads->graph[*ei]->getLength() * roads->graph[*ei]->lanes;
		}

		if (weight > grid_weight[u][v]) {
			grid_weight[u][v] = weight;
			grid_desc[u][v] = *vi;
		}
	}

	// グリッド情報に基づいて、道路網を作成する
	RoadGraph* new_roads = new RoadGraph();
	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// まずは、頂点のみを作成
	for (int i = 0; i < grid_desc.size(); i++) {
		for (int j = 0; j < grid_desc[i].size(); j++) {
			if (grid_weight[i][j] == 0.0f) continue;

			RoadVertex* v = new RoadVertex(orig + QVector2D(cellLength, 0) * j + QVector2D(0, cellLength) * i);
			RoadVertexDesc v_desc = boost::add_vertex(new_roads->graph);
			new_roads->graph[v_desc] = v;

			conv[grid_desc[i][j]] = v_desc;
		}
	}

	// 次に、エッジを作成
	for (int i = 0; i < grid_desc.size(); i++) {
		for (int j = 0; j < grid_desc[i].size(); j++) {
			if (grid_weight[i][j] == 0.0f) continue;

			if (j < grid_desc[i].size() - 1) {
				// 右隣とのエッジをチェック
				if (grid_weight[i][j+1] > 0.0f) {
					RoadVertexDesc orig_v1_desc = grid_desc[i][j];
					RoadVertexDesc orig_v2_desc = grid_desc[i][j+1];
					if (isConnected(roads, orig_v1_desc, orig_v2_desc)) {
						addEdge(new_roads, conv[orig_v1_desc], conv[orig_v2_desc], 2, 2);	// to be updated!
					}
				}
			}

			if (j > 0) {
				// 左隣とのエッジをチェック
				if (grid_weight[i][j-1] > 0.0f) {
					RoadVertexDesc orig_v1_desc = grid_desc[i][j];
					RoadVertexDesc orig_v2_desc = grid_desc[i][j-1];
					if (isConnected(roads, orig_v1_desc, orig_v2_desc)) {
						addEdge(new_roads, conv[orig_v1_desc], conv[orig_v2_desc], 2, 2);	// to be updated!
					}
				}
			}

			if (i < grid_desc.size()) {
				// 上隣とのエッジをチェック
				if (grid_weight[i+1][j] > 0.0f) {
					RoadVertexDesc orig_v1_desc = grid_desc[i][j];
					RoadVertexDesc orig_v2_desc = grid_desc[i+1][j];
					if (isConnected(roads, orig_v1_desc, orig_v2_desc)) {
						addEdge(new_roads, conv[orig_v1_desc], conv[orig_v2_desc], 2, 2);	// to be updated!
					}
				}
			}

			if (i > 0) {
				// 下隣とのエッジをチェック
				if (grid_weight[i-1][j] > 0.0f) {
					RoadVertexDesc orig_v1_desc = grid_desc[i][j];
					RoadVertexDesc orig_v2_desc = grid_desc[i-1][j];
					if (isConnected(roads, orig_v1_desc, orig_v2_desc)) {
						addEdge(new_roads, conv[orig_v1_desc], conv[orig_v2_desc], 2, 2);	// to be updated!
					}
				}
			}
		}
	}

	return new_roads;
}

/**
 * 道路網が指定されたareaにおさまるようにする。
 */
void GraphUtil::scaleToBBox(RoadGraph* roads, BBox& area) {
	BBox curArea = getAABoundingBox(roads);
	QVector2D scale(area.dx() / curArea.dx(), area.dy() / curArea.dy());

	// 全ての頂点を、指定したareaに入るよう移動する
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		QVector2D pos = roads->graph[*vi]->pt - curArea.midPt();
		pos.setX(pos.x() * scale.x());
		pos.setY(pos.y() * scale.y());
		pos += area.midPt();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, roads->graph);
			if (!roads->graph[tgt]->valid) continue;

			moveEdge(roads, *ei, pos, roads->graph[tgt]->pt);
		}

		roads->graph[*vi]->pt = pos;
	}
}

/**
 * バネの原理を使って、道路網のエッジの長さを均等にする。
 *
 * まず、全エッジの平均長を計算し、これをエッジの本来の長さと仮定する。
 * 次に、各頂点について、各隣接エッジの長さの、本来長からの変形量を使って、各頂点にかかる仮想的な力を計算する。
 * 最後に、この仮想的な力に、適当なdTをかけた値を使って、各頂点を移動させる。
 * これを、一定数、繰り返す。（終了条件について、要検討）
 */
void GraphUtil::normalizeBySpring(RoadGraph* roads, BBox& area) {
	// バネの原理を使って、各エッジの長さを均等にする
	float step = 0.03f;

	//for (int i = 0; i < 1000; i++) {
	for (int i = 0; i < 1; i++) {
		float avg_edge_length = computeAvgEdgeLength(roads);

		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
			if (!roads->graph[*vi]->valid) continue;

			// 頂点にかかる力を計算する
			QVector2D force;

			// 隣接エッジからは、引っ張られる
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
				if (!roads->graph[*ei]->valid) continue;

				RoadVertexDesc src = *vi;
				RoadVertexDesc tgt = boost::target(*ei, roads->graph);

				QVector2D dir = roads->graph[tgt]->pt - roads->graph[src]->pt;
				float x = (roads->graph[tgt]->pt - roads->graph[src]->pt).length() - avg_edge_length * 1.0f;
				
				force += dir.normalized() * x;
			}

			// 接続されていない、近隣の頂点からは、反発力を受ける
			RoadVertexIter vi2, vend2;
			for (boost::tie(vi2, vend2) = boost::vertices(roads->graph); vi2 != vend2; ++vi2) {
				if (!roads->graph[*vi2]->valid) continue;

				if (hasEdge(roads, *vi, *vi2)) continue;

				/*
				QVector2D dir = roads->graph[*vi]->pt - roads->graph[*vi2]->pt;
				float x = avg_edge_length * 1.44f - (roads->graph[*vi2]->pt - roads->graph[*vi]->pt).length();

				if (x > 0.0f) {
					force += dir.normalized() * x;
				}*/

				RoadVertexDesc src = *vi;
				RoadVertexDesc tgt = *vi2;

				QVector2D dir = roads->graph[tgt]->pt - roads->graph[src]->pt;
				float x = (roads->graph[tgt]->pt - roads->graph[src]->pt).length() - avg_edge_length * 1.0f;
				
				force += dir.normalized() * x * 0.02f;	// ←　この係数は、微調整が必要。。。
			}

			// 移動後の位置が、指定された範囲内か、チェック
			QVector2D pos = roads->graph[*vi]->pt + force * step;
			if (area.contains(pos)) {
				// 頂点をする
				roads->graph[*vi]->pt = pos;

				// エッジも移動する
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
					if (!roads->graph[*ei]->valid) continue;

					RoadVertexDesc tgt = boost::target(*ei, roads->graph);

					moveEdge(roads, *ei, pos, roads->graph[tgt]->pt);
				}
			}
		}
	}
}

/**
 * 頂点AB間に複数のエッジがある場合、１つ目以外は無効にする。
 */
bool GraphUtil::removeDuplicateEdges(RoadGraph* roads) {
	bool removed = false;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		QList<RoadVertexDesc> targets;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, roads->graph);

			if (targets.contains(tgt)) {
				roads->graph[*ei]->valid = false;
				removed = true;
			} else {
				targets.push_back(tgt);
			}
		}
	}
	
	return removed;
}

/**
 * Deadendのエッジについて、近くに頂点がある場合は、Snapさせる。
 * まず、degree>1の頂点のみを対象として、最も近い頂点を探す。
 * もし存在しない場合は、degree=1の頂点のみを対象として、最も近い頂点を探す。
 * ただし、Snap対象となるエッジとのなす角度がmin_angle_threshold以下の場合は、対象外。
 */
void GraphUtil::snapDeadendEdges(RoadGraph* roads, float threshold) {
	float min_angle_threshold = 0.34f;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (!roads->graph[*vi]->valid) continue;

		// degreeが1の頂点以外は、対象外
		if (GraphUtil::getDegree(roads, *vi) != 1) continue;

		// 当該頂点と接続されている唯一の頂点を取得
		RoadVertexDesc tgt;
		RoadEdgeDesc e_desc;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			tgt = boost::target(*ei, roads->graph);
			e_desc = *ei;
			break;
		}

		// 近接頂点を探す（ただし、degree=1の頂点は、対象外）
		RoadVertexDesc nearest_desc;
		float min_dist = std::numeric_limits<float>::max();

		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(roads->graph); vi2 != vend2; ++vi2) {
			if (!roads->graph[*vi2]->valid) continue;
			if (*vi2 == *vi) continue;
			if (*vi2 == tgt) continue;
			if (GraphUtil::getDegree(roads, *vi2) == 1) continue;

			float dist = (roads->graph[*vi2]->pt - roads->graph[*vi]->pt).length();

			// 近接頂点が、*viよりもtgtの方に近い場合は、当該近接頂点は対象からはずす
			float dist2 = (roads->graph[*vi2]->pt - roads->graph[tgt]->pt).length();
			if (dist > dist2) continue;

			if (dist < min_dist) {
				nearest_desc = *vi2;
				min_dist = dist;
			}

			// *vi2から出るエッジとのなす角度の最小値が小さすぎる場合は、対象からはずす
			float min_angle = std::numeric_limits<float>::max();
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi2, roads->graph); ei != eend; ++ei) {
				if (!roads->graph[*ei]->valid) continue;

				RoadVertexDesc tgt2 = boost::target(*ei, roads->graph);
				float angle = GraphUtil::diffAngle(roads->graph[*vi]->pt - roads->graph[tgt]->pt, roads->graph[*vi2]->pt - roads->graph[tgt2]->pt);
				if (angle < min_angle) {
					min_angle = angle;
				}
			}
			if (min_angle < min_angle_threshold) continue;
		}

		// 見つからない場合は、degree=1の頂点の中から、最も近い頂点を探す
		if (min_dist > threshold) {
			for (boost::tie(vi2, vend2) = boost::vertices(roads->graph); vi2 != vend2; ++vi2) {
				if (!roads->graph[*vi2]->valid) continue;
				if (*vi2 == *vi) continue;
				if (*vi2 == tgt) continue;
				if (GraphUtil::getDegree(roads, *vi2) != 1) continue;

				// 最も近い頂点の、唯一のエッジを探す
				RoadEdgeDesc e_desc2;
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(*vi2, roads->graph); ei != eend; ++ei) {
					if (!roads->graph[*ei]->valid) continue;

					e_desc2 = *ei;
					break;
				}

				// 最も近い頂点のエッジが短すぎる場合は、スキップ
				//if (roads->graph[e_desc2]->getLength() < deadend_removal_threshold) continue;

				float dist = (roads->graph[*vi2]->pt - roads->graph[*vi]->pt).length();

				// 近接頂点が、*viよりもtgtの方に近い場合は、当該近接頂点は対象からはずす
				float dist2 = (roads->graph[*vi2]->pt - roads->graph[tgt]->pt).length();
				if (dist > dist2) continue;

				if (dist < min_dist) {
					nearest_desc = *vi2;
					min_dist = dist;
				}

				// *vi2から出るエッジとのなす角度の最小値が小さすぎる場合は、対象からはずす
				float min_angle = std::numeric_limits<float>::max();
				for (boost::tie(ei, eend) = boost::out_edges(*vi2, roads->graph); ei != eend; ++ei) {
					if (!roads->graph[*ei]->valid) continue;

					RoadVertexDesc tgt2 = boost::target(*ei, roads->graph);
					float angle = GraphUtil::diffAngle(roads->graph[*vi]->pt - roads->graph[tgt]->pt, roads->graph[*vi2]->pt - roads->graph[tgt2]->pt);
					if (angle < min_angle) {
						min_angle = angle;
					}
				}
				if (min_angle < min_angle_threshold) continue;
			}
		}

		// 当該頂点と近接頂点との距離が、snap_deadend_threshold未満か？
		if (min_dist <= threshold) {
			// 一旦、古いエッジを、近接頂点にスナップするよう移動する
			GraphUtil::moveEdge(roads, e_desc, roads->graph[nearest_desc]->pt, roads->graph[tgt]->pt);

			if (GraphUtil::hasEdge(roads, nearest_desc, tgt, false)) {
				// もともとエッジがあるが無効となっている場合、それを有効にし、エッジのポリラインを更新する
				RoadEdgeDesc new_e_desc = GraphUtil::getEdge(roads, nearest_desc, tgt, false);
				roads->graph[new_e_desc]->valid = true;
				roads->graph[new_e_desc]->polyLine = roads->graph[e_desc]->polyLine;
			} else {
				// 該当頂点間にエッジがない場合は、新しいエッジを追加する
				GraphUtil::addEdge(roads, nearest_desc, tgt, roads->graph[e_desc]);
			}

			// 古いエッジを無効にする
			roads->graph[e_desc]->valid = false;

			// 当該頂点を無効にする
			roads->graph[*vi]->valid = false;
		}
	}
}

/**
 * ２つのデータリストの差の最小値を返却する。
 * 各データは、１回のみ比較に使用できる。
 */
float GraphUtil::computeMinDiffAngle(std::vector<float> *data1, std::vector<float> *data2) {
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
float GraphUtil::diffAngle(const QVector2D& dir1, const QVector2D& dir2) {
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
float GraphUtil::computeMinUnsimilarity(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& best_map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& best_map2) {
	int N = getNumVertices(roads1);
	int M = getNumVertices(roads2);

	roads1->computeConnectivities();
	roads2->computeConnectivities();

	std::vector<int> corr1;
	//std::vector<int> corr2;
	for (int i = 0; i < N; i++) {
		corr1.push_back(0);
	}
	/*
	for (int i = 0; i < M; i++) {
		corr2.push_back(0);
	}
	*/

	float min_diff = std::numeric_limits<float>::max();
	std::vector<int> min_corr1;
	std::vector<int> min_corr2;

	while (true) {
		QMap<RoadVertexDesc, RoadVertexDesc> map1;
		QMap<RoadVertexDesc, RoadVertexDesc> map2;

		// corr1、corr2を元に、mapを生成する
		for (int i = 0; i < N; i++) {
			map1[i] = corr1[i];
			map2[map1[i]] = i;
		}

		// map2で相手のいないノードについて、
		/*
		bool updated = true;
		while (updated) {
			updated = false;

			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
				if (map2.contains(*vi)) continue;

				// 最も近い隣接ノードを探す
				float min_dist = std::numeric_limits<float>::max();	
				RoadVertexDesc min_tgt;
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads2->graph); ei != eend; ++ei) {
					RoadVertexDesc tgt = boost::target(*ei, roads2->graph);
					if (!map2.contains(tgt)) continue;

					float dist = roads2->graph[*ei]->getLength();
					if (dist < min_dist) {
						min_dist = dist;
						min_tgt = tgt;
					}
				}

				// 隣接ノードの相手を、当該ノード(*vi)の相手とする
				if (min_dist < std::numeric_limits<float>::max()) {
					map2[*vi] = map2[min_tgt];
					updated = true;
					break;
				}
			}
		}
		*/

		// 与えられたマッピングについて、非類似度を計算する
		float diff = computeDissimilarity(roads1, map1, roads2, map2, 1.0f, 1.0f, 1.0f, 1.0f);
		if (diff < min_diff) {
			min_diff = diff;
			best_map1 = map1;
			best_map2 = map2;
		}
		
		if (!nextSequence(corr1, M)) break;
	}

	return min_diff;
}

/**
 * 対応する頂点が与えられている時に、２つの道路網のトポロジーの違いを数値化して返却する。
 * トポロジーの違いなので、座標は一切関係ない。隣接ノードとの接続性のみを考慮する。
 *
 * @param w_connectivity		対応するエッジがない場合のペナルティ
 * @param w_split				対応が重複している場合のペナルティ
 * @param w_angle				エッジの角度のペナルティ
 * @param w_distance			対応する頂点の距離に対するペナルティ
 */
float GraphUtil::computeDissimilarity(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, float w_connectivity, float w_split, float w_angle, float w_distance) {
	float penalty = 0.0f;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// コネクティビティに基づいたペナルティの計上
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		if (map1.contains(*vi)) {
			RoadVertexDesc v2 = map1[*vi];

			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads1->graph); ei != eend; ++ei) {
				if (!roads1->graph[*ei]->valid) continue;

				RoadVertexDesc v1b = boost::target(*ei, roads1->graph);
				RoadVertexDesc v2b = map1[v1b];

				if (v2 == v2b || !isConnected(roads2, v2, v2b)) { // 対応ノード間が接続されてない場合
				//if (v2 == v2b || !roads2->isConnected(v2, v2b)) { // キャッシュによる高速化（ただし、事前にconnectivityを計算する必要有り
					penalty += roads1->graph[*ei]->getLength() * roads1->graph[*ei]->weight * w_connectivity;
				} else {
					QVector2D dir1 = roads1->graph[v1b]->getPt() - roads1->graph[*vi]->getPt();
					QVector2D dir2 = roads2->graph[v2b]->getPt() - roads2->graph[v2]->getPt();
					if (dir1.lengthSquared() > 0.0f && dir2.lengthSquared() > 0.0f) {
						penalty += diffAngle(dir1, dir2) * w_angle;
					} else {
						// どちらかのエッジの長さ＝０、つまり、エッジがないので、コネクティビティのペナルティを課す
						// 道路網１の方のエッジの長さが０の場合、ペナルティは０となるが、
						// 道路網２の計算の際に、ペナルティが加算されるので、良いだろう。
						penalty += roads1->graph[*ei]->getLength() * roads1->graph[*ei]->weight * w_connectivity;
					}
				}
			}
		} else { // 当該ノードに対応するノードがない場合は、全てのエッジをペナルティとして計上する
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads1->graph); ei != eend; ++ei) {
				if (!roads1->graph[*ei]->valid) continue;

				RoadVertexDesc v1b = boost::target(*ei, roads1->graph);

				penalty += roads1->graph[*ei]->getLength() * roads1->graph[*ei]->weight * w_connectivity;
			}
		}
	}

	for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
		if (!roads2->graph[*vi]->valid) continue;

		if (map2.contains(*vi)) {
			RoadVertexDesc v1 = map2[*vi];

			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads2->graph); ei != eend; ++ei) {
				if (!roads2->graph[*ei]->valid) continue;

				RoadVertexDesc v2b = boost::target(*ei, roads2->graph);
				RoadVertexDesc v1b = map2[v2b];

				if (v1 == v1b || !isConnected(roads1, v1, v1b)) { // 対応ノード間が接続されてない場合
				//if (v1 == v1b || !roads1->isConnected(v1, v1b)) { // キャッシュによる高速化（ただし、事前にconnectivityを計算する必要有り
					penalty += roads2->graph[*ei]->getLength() * roads2->graph[*ei]->weight * w_connectivity;
				} else {
					QVector2D dir1 = roads1->graph[v1b]->getPt() - roads1->graph[v1]->getPt();
					QVector2D dir2 = roads2->graph[v2b]->getPt() - roads2->graph[*vi]->getPt();
					if (dir1.lengthSquared() > 0.0f && dir2.lengthSquared() > 0.0f) {
						penalty += diffAngle(dir1, dir2) * w_angle;
					} else {
						// どちらかのエッジの長さ＝０、つまり、エッジがないので、コネクティビティのペナルティを課す
						// 道路網２の方のエッジの長さが０の場合、ペナルティは０となるが、
						// 道路網１の計算の際に、ペナルティが加算されるので、良いだろう。
						penalty += roads2->graph[*ei]->getLength() * roads2->graph[*ei]->weight * w_connectivity;
					}
				}
			}
		} else { // 当該ノードに対応するノードがない場合は、全てのエッジをペナルティとして計上する
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads2->graph); ei != eend; ++ei) {
				if (!roads2->graph[*ei]->valid) continue;

				RoadVertexDesc v2b = boost::target(*ei, roads2->graph);

				penalty += roads2->graph[*ei]->getLength() * roads2->graph[*ei]->weight * w_connectivity;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 重複マッチング（モーフィングの際に、スプリットが発生）によるペナルティの計上
	QSet<RoadVertexDesc> used;
	for (QMap<RoadVertexDesc, RoadVertexDesc>::iterator it = map1.begin(); it != map1.end(); ++it) {
		if (used.contains(it.value())) {
			penalty += w_split;
		} else {
			used.insert(it.value());
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 頂点に距離に関するペナルティの計上
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		if (!roads1->graph[*vi]->valid) continue;

		if (map1.contains(*vi)) {
			RoadVertexDesc v2 = map1[*vi];

			penalty+= (roads1->graph[*vi]->pt - roads2->graph[v2]->pt).length() * w_distance;
		} else {
			// 対応する頂点がない場合、ペナルティはなし？
		}
	}

	return penalty;
}

/**
 * 対応する頂点が与えられている時に、２つの道路網のトポロジーの違いを数値化して返却する。
 * トポロジーの違いなので、座標は一切関係ない。隣接ノードとの接続性のみを考慮する。
 *
 * @param w_matching			対応するエッジがない場合のペナルティ
 */
float GraphUtil::computeDissimilarity2(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2, float w_matching, float w_split, float w_angle, float w_distance) {
	float penalty = 0.0f;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 道路網１の各エッジについて、対応エッジがない場合のペナルティ
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		if (!roads1->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads1->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads1->graph);
		
		if (!map1.contains(src) || !map1.contains(tgt)) {
			// 対応エッジがないので、当該エッジのImportanceに基づいて、ペナルティを追加
			penalty += roads1->graph[*ei]->importance * w_matching;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// 道路網２の各エッジについて、対応エッジがない場合のペナルティ
	for (boost::tie(ei, eend) = boost::edges(roads2->graph); ei != eend; ++ei) {
		if (!roads2->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads2->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads2->graph);
		
		if (!map2.contains(src) || !map2.contains(tgt)) {
			// 対応エッジがないので、当該エッジのImportanceに基づいて、ペナルティを追加
			penalty += roads2->graph[*ei]->importance * w_matching;
		}
	}

	return penalty;
}

/**
 * NearestNeighborに基づいて、２つの道路網のマッチングを行う。
 */
void GraphUtil::findCorrespondenceByNearestNeighbor(RoadGraph* roads1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map1, QMap<RoadVertexDesc, RoadVertexDesc>& map2) {
	if (getNumVertices(roads1) < getNumVertices(roads2)) {
		// 道路網１の各頂点に対して、対応する道路網２の頂点を探す
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
			if (!roads1->graph[*vi]->valid) continue;

			RoadVertexDesc v2 = findNearestVertex(roads2, roads1->graph[*vi]->pt);
			map1[*vi] = v2;
			map2[v2] = *vi;
		}

		// 道路網２の各頂点に対して、まだペアがない場合は、対応する道路網１の頂点を探す
		for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
			if (!roads2->graph[*vi]->valid) continue;

			if (map2.contains(*vi)) continue;

			RoadVertexDesc v1 = findNearestVertex(roads1, roads2->graph[*vi]->pt);
			map2[*vi] = v1;
		}
	} else {
		// 道路網２の各頂点に対して、対応する道路網１の頂点を探す
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads2->graph); vi != vend; ++vi) {
			if (!roads2->graph[*vi]->valid) continue;

			RoadVertexDesc v1 = findNearestVertex(roads1, roads2->graph[*vi]->pt);
			map2[*vi] = v1;
			map1[v1] = *vi;
		}

		// 道路網１の各頂点に対して、まだペアがない場合は、対応する道路網２の頂点を探す
		for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
			if (!roads1->graph[*vi]->valid) continue;

			if (map1.contains(*vi)) continue;

			RoadVertexDesc v2 = findNearestVertex(roads2, roads1->graph[*vi]->pt);
			map1[*vi] = v2;
		}
	}
}

/**
 * ２つの対応するノードについて、その子ノードのベストマッチングを探す。
 * アルゴリズム：角度の差の最大値を最小とする組合せを探す。
 * ただし、degreeが6を超えると、計算量が爆発するので、ループ数を1000に制約する。
 */
QMap<RoadVertexDesc, RoadVertexDesc> GraphUtil::findCorrespondentEdges(RoadGraph* roads1, RoadVertexDesc parent1, std::vector<RoadVertexDesc> children1, RoadGraph* roads2, RoadVertexDesc parent2, std::vector<RoadVertexDesc> children2) {
	QMap<RoadVertexDesc, RoadVertexDesc> map;

	std::vector<int> permutation;
	float min_diff = std::numeric_limits<float>::max();

	if (children1.size() <= children2.size()) {
		for (int i = 0; i < children2.size(); i++) {
			permutation.push_back(i);
		}

		for (int count = 0; count < 1000; count++) {
			// 角度の差の最大値を求める
			float diff = 0.0f;
			for (int i = 0; i < children1.size(); i++) {
				RoadEdgeDesc e1 = getEdge(roads1, parent1, children1[i]);
				RoadEdgeDesc e2 = getEdge(roads2, parent2, children2[permutation[i]]);

				QVector2D dir1;
				if ((roads1->graph[parent1]->pt - roads1->graph[e1]->polyLine[0]).length() < (roads1->graph[parent1]->pt - roads1->graph[e1]->polyLine[roads1->graph[e1]->polyLine.size() - 1]).length()) {
					dir1 = roads1->graph[e1]->polyLine[1] - roads1->graph[e1]->polyLine[0];
				} else {
					dir1 = roads1->graph[e1]->polyLine[roads1->graph[e1]->polyLine.size() - 2] - roads1->graph[e1]->polyLine[roads1->graph[e1]->polyLine.size() - 1];
				}

				QVector2D dir2;
				if ((roads2->graph[parent2]->pt - roads2->graph[e2]->polyLine[0]).length() < (roads2->graph[parent2]->pt - roads1->graph[e2]->polyLine[roads2->graph[e2]->polyLine.size() - 1]).length()) {
					dir2 = roads2->graph[e2]->polyLine[1] - roads2->graph[e2]->polyLine[0];
				} else {
					dir2 = roads2->graph[e2]->polyLine[roads2->graph[e2]->polyLine.size() - 2] - roads2->graph[e2]->polyLine[roads2->graph[e2]->polyLine.size() - 1];
				}

				/*
				QVector2D dir1 = roads1->graph[children1[i]]->pt - roads1->graph[parent1]->pt;
				QVector2D dir2 = roads2->graph[children2[permutation[i]]]->pt - roads2->graph[parent2]->pt;
				*/

				diff = std::max(diff, diffAngle(dir1, dir2));
			}

			if (diff < min_diff) {
				min_diff = diff;
				map.clear();
				for (int i = 0; i < children1.size(); i++) {
					map[children1[i]] = children2[permutation[i]];
				}
			}

			if (!std::next_permutation(permutation.begin(), permutation.end())) break;
		}
	} else {
		for (int i = 0; i < children1.size(); i++) {
			permutation.push_back(i);
		}

		for (int count = 0; count < 1000; count++) {
			// 角度の差の最大値を求める
			float diff = 0.0f;
			for (int i = 0; i < children2.size(); i++) {
				QVector2D dir1 = roads1->graph[children1[permutation[i]]]->pt - roads1->graph[parent1]->pt;
				QVector2D dir2 = roads2->graph[children2[i]]->pt - roads2->graph[parent2]->pt;

				diff = std::max(diff, diffAngle(dir1, dir2));
			}

			if (diff < min_diff) {
				min_diff = diff;
				map.clear();
				for (int i = 0; i < children2.size(); i++) {
					map[children1[permutation[i]]] = children2[i];
				}
			}

			if (!std::next_permutation(permutation.begin(), permutation.end())) break;
		}
	}

	return map;
}

/**
 * 与えられた数列の、先頭の値を１インクリメントする。
 * N進法なので、Nになったら、桁が繰り上がる。つまり、次の要素の値を１インクリメントする。
 * ex. {1, 2, 3} => {2, 2, 3}
 * ex. {N-1, 3, 3} => {0, 4, 3}
 * ex. {N-1, N-1, 3} => {0, 0, 4}
 */
bool GraphUtil::nextSequence(std::vector<int>& seq, int N) {
	int index = 0;
	while (true) {
		if (seq[index] < N - 1) break;

		seq[index] = 0;

		if (++index >= seq.size()) break;
	}

	if (index < seq.size()) {
		seq[index]++;
		return true;
	} else {
		return false;
	}
}

/**
 * エッジの平均長を計算して返却する。
 */
float GraphUtil::computeAvgEdgeLength(RoadGraph* roads) {
	float length = 0.0f;
	int count = 0;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (!roads->graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads->graph);
		RoadVertexDesc tgt = boost::target(*ei, roads->graph);

		length += (roads->graph[src]->pt - roads->graph[tgt]->pt).length();
		count++;
	}

	if (count == 0) return 0;
	else return length / (float)count;
}

/**
 * ２つの道路網について、似ているエッジペアを類似度でソートして、トップNを返却する。
 */
QList<EdgePair> GraphUtil::getClosestEdgePairs(RoadGraph* roads1, RoadGraph* roads2, int num) {
	QList<EdgePair> pairs;

	// Importance順に並べたエッジリストを取得
	QList<RoadEdgeDesc> edges1 = roads1->getOrderedEdgesByImportance();
	QList<RoadEdgeDesc> edges2 = roads2->getOrderedEdgesByImportance();

	while (pairs.size() < num && !edges1.empty() && !edges2.empty()) {
		float min_diff = std::numeric_limits<float>::max();
		RoadEdgeDesc e1;
		RoadEdgeDesc e2;

		// 道路網１のTop10 Importantエッジに対して
		int topN = std::min(10, edges1.size());
		for (int i = 0; i < topN; i++) {
			// 似ている対応エッジを道路網２から探す
			for (int j = 0; j < edges2.size(); j++) {
				float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[i], roads2, edges2[j]);
				if (diff < min_diff) {
					min_diff = diff;
					e1 = edges1[i];
					e2 = edges2[j];
				}
			}
		}

		// 道路網２のTop10 Importantエッジに対して
		topN = std::min(10, edges2.size());
		for (int i = 0; i < topN; i++) {
			// 似ている対応エッジを道路網１ら探す
			for (int j = 0; j < edges1.size(); j++) {
				float diff = GraphUtil::computeDissimilarityOfEdges(roads1, edges1[j], roads2, edges2[i]);
				if (diff < min_diff) {
					min_diff = diff;
					e1 = edges1[j];
					e2 = edges2[i];
				}
			}
		}

		// 抽出された最も似ているエッジペアを、リストに登録する
		pairs.push_back(EdgePair(e1, e2));

		// 選択されたペアから、両端のノードを取得
		RoadVertexDesc src1 = boost::source(e1, roads1->graph);
		RoadVertexDesc tgt1 = boost::target(e1, roads1->graph);
		RoadVertexDesc src2 = boost::source(e2, roads2->graph);
		RoadVertexDesc tgt2 = boost::target(e2, roads2->graph);

		//エッジリストから、使用したエッジに含まれる頂点を含むエッジを削除
		for (int j = edges1.size() - 1; j >= 0; j--) {
			if (boost::source(edges1[j], roads1->graph) == src1 || boost::source(edges1[j], roads1->graph) == tgt1 || boost::target(edges1[j], roads1->graph) == src1 || boost::target(edges1[j], roads1->graph) == tgt1) {
				edges1.removeAt(j);
			}
		}
		for (int j = edges2.size() - 1; j >= 0; j--) {
			if (boost::source(edges2[j], roads2->graph) == src2 || boost::source(edges2[j], roads2->graph) == tgt2 || boost::target(edges2[j], roads2->graph) == src2 || boost::target(edges2[j], roads2->graph) == tgt2) {
				edges2.removeAt(j);
			}
		}
	}

	return pairs;
}

/**
 * グローバル Rigid ICPを実施し、道路網１の位置を、道路網２に近づくよう変形する。
 */
void GraphUtil::rigidICP(RoadGraph* roads1, RoadGraph* roads2, QList<EdgePair>& pairs) {
	cv::Mat src(pairs.size() * 2, 2, CV_32FC2);
	cv::Mat dst(pairs.size() * 2, 2, CV_32FC2);

	for (int i = 0; i < pairs.size(); i++) {
		RoadEdgeDesc e1 = pairs[i].edge1;
		RoadEdgeDesc e2 = pairs[i].edge2;

		// エッジの両端頂点を取得
		RoadVertexDesc src1 = boost::source(e1, roads1->graph);
		RoadVertexDesc tgt1 = boost::target(e1, roads1->graph);
		RoadVertexDesc src2 = boost::source(e2, roads2->graph);
		RoadVertexDesc tgt2 = boost::target(e2, roads2->graph);

		// もしsrc1-tgt2、tgt1-src2の方が近かったら、src2とtgt2を入れ替える
		if ((roads1->graph[src1]->pt - roads2->graph[src2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[tgt2]->pt).length() > (roads1->graph[src1]->pt - roads2->graph[tgt2]->pt).length() + (roads1->graph[tgt1]->pt - roads2->graph[src2]->pt).length()) {
			src2 = boost::target(e2, roads2->graph);
			tgt2 = boost::source(e2, roads2->graph);
		}

		// 頂点の座標を行列に格納
		src.at<float>(i * 2, 0) = roads1->graph[src1]->pt.x();
		src.at<float>(i * 2, 1) = roads1->graph[src1]->pt.y();
		src.at<float>(i * 2 + 1, 0) = roads1->graph[tgt1]->pt.x();
		src.at<float>(i * 2 + 1, 1) = roads1->graph[tgt1]->pt.y();
		dst.at<float>(i * 2, 0) = roads2->graph[src2]->pt.x();
		dst.at<float>(i * 2, 1) = roads2->graph[src2]->pt.y();
		dst.at<float>(i * 2 + 1, 0) = roads2->graph[tgt2]->pt.x();
		dst.at<float>(i * 2 + 1, 1) = roads2->graph[tgt2]->pt.y();
	}

	// Rigid ICP 変換行列を計算
	cv::Mat transformMat = cv::estimateRigidTransform(src, dst, true);

	// 道路網１の頂点座標を、変換行列を使って更新
	src = convertVerticesToCVMatrix(roads1, false);
	cv::Mat src2;
	cv::transform(src, src2, transformMat);

	// 道路網１の頂点座標を実際に更新する
	RoadVertexIter vi, vend;
	int count = 0;
	for (boost::tie(vi, vend) = boost::vertices(roads1->graph); vi != vend; ++vi) {
		roads1->graph[*vi]->pt.setX(src2.at<float>(count, 0));
		roads1->graph[*vi]->pt.setY(src2.at<float>(count, 1));
		count++;
	}

	// 道路網１のエッジの座標も更新する
	src = convertEdgesToCVMatrix(roads1, false);
	cv::transform(src, src2, transformMat);

	// 道路網１のエッジ座標を実際に更新する
	RoadEdgeIter ei, eend;
	count = 0;
	for (boost::tie(ei, eend) = boost::edges(roads1->graph); ei != eend; ++ei) {
		for (int i = 0; i < roads1->graph[*ei]->polyLine.size(); i++) {
			roads1->graph[*ei]->polyLine[i].setX(src2.at<float>(count, 0));
			roads1->graph[*ei]->polyLine[i].setY(src2.at<float>(count, 1));
			count++;
		}
	}
}

/**
 * 道路網の頂点座標を、Nx2の行列に変換する
 */
cv::Mat GraphUtil::convertVerticesToCVMatrix(RoadGraph* roads, bool onlyValidVertex) {
	cv::Mat ret(getNumVertices(roads, onlyValidVertex), 2, CV_32FC2);

	RoadVertexIter vi, vend;
	int count = 0;
	for (boost::tie(vi, vend) = boost::vertices(roads->graph); vi != vend; ++vi) {
		if (onlyValidVertex && !roads->graph[*vi]->valid) continue;

		ret.at<float>(count, 0) = roads->graph[*vi]->pt.x();
		ret.at<float>(count, 1) = roads->graph[*vi]->pt.y();
		count++;
	}

	return ret;
}

/**
 * 道路網のエッジ座標を、Nx2の行列に変換する
 */
cv::Mat GraphUtil::convertEdgesToCVMatrix(RoadGraph* roads, bool onlyValidVertex) {
	std::vector<QVector2D> data;
	
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
		if (onlyValidVertex && !roads->graph[*ei]->valid) continue;

		for (int i = 0; i < roads->graph[*ei]->polyLine.size(); i++) {
			data.push_back(roads->graph[*ei]->polyLine[i]);
		}
	}
	
	cv::Mat ret(data.size(), 2, CV_32FC2);

	for (int i = 0; i < data.size(); i++) {
		ret.at<float>(i, 0) = data[i].x();
		ret.at<float>(i, 1) = data[i].y();
	}

	return ret;
}

/**
 * グリッドスタイルの道路網を作成する。
 *
 * @param size		一辺の長さ [m]
 * @param num		一辺のノード数
 */
RoadGraph* GraphUtil::createGridNetwork(float size, int num) {
	RoadGraph* roads = new RoadGraph();

	// 各エッジの長さ
	float length = size / (float)(num - 1);

	// 原点座標
	QVector2D orig(-size / 2.0f, -size / 2.0f);

	// ノードを作成
	for (int i = 0; i < num - 2; i++) {
		for (int j = 0; j < num; j++) {
			RoadVertex* v = new RoadVertex(orig + QVector2D(j * length, i * length + length));
			RoadVertexDesc desc = boost::add_vertex(roads->graph);
			roads->graph[desc] = v;
		}
	}
	for (int i = 0; i < num - 2; i++) {
		RoadVertex* v = new RoadVertex(orig + QVector2D(i * length + length, 0));
		RoadVertexDesc desc = boost::add_vertex(roads->graph);
		roads->graph[desc] = v;
	}
	for (int i = 0; i < num - 2; i++) {
		RoadVertex* v = new RoadVertex(orig + QVector2D(i * length + length, size));
		RoadVertexDesc desc = boost::add_vertex(roads->graph);
		roads->graph[desc] = v;
	}

	// エッジを作成
	for (int i = 0; i < num - 2; i++) {
		for (int j = 0; j < num - 1; j++) {
			addEdge(roads, i * num + j, i * num + j + 1, 2, 2);
		}
	}
	for (int i = 0; i < num - 3; i++) {
		for (int j = 0; j < num - 2; j++) {
			addEdge(roads, i * num + 1 + j, i * num + 1 + j + num, 2, 2);
		}
	}
	for (int i = 0; i < num - 2; i++) {
		addEdge(roads, num * (num - 2) + i, i + 1, 2, 2);
		addEdge(roads, num * (num - 2) + (num - 2) + i, num * (num - 3) + i + 1, 2, 2);
	}

	return roads;
}

/**
 * 曲がったスタイルの道路網を作成する。
 *
 * @param size		一辺の長さ [m]
 * @param num		一辺のノード数
 * @param angle		傾ける角度 [rad]
 */
RoadGraph* GraphUtil::createCurvyNetwork(float size, int num, float angle) {
	RoadGraph* roads = new RoadGraph();

	// 各エッジの長さ
	float length = size / (float)(num - 1);

	// 原点座標
	QVector2D orig(-size / 2.0f, -size / 2.0f);

	// ノードを作成
	for (int i = 0; i < num - 2; i++) {
		for (int j = 0; j < num; j++) {
			QVector2D pos = orig + QVector2D(j * length, i * length + length);
			QVector2D pos2;
			pos2.setX(pos.x() * cosf(angle) - pos.y() * sinf(angle));
			pos2.setY(pos.x() * sinf(angle) + pos.y() * cosf(angle));
			RoadVertex* v = new RoadVertex(pos2);
			RoadVertexDesc desc = boost::add_vertex(roads->graph);
			roads->graph[desc] = v;
		}
	}
	for (int i = 0; i < num - 2; i++) {
		QVector2D pos = orig + QVector2D(i * length + length, 0);
		QVector2D pos2;
		pos2.setX(pos.x() * cosf(angle) - pos.y() * sinf(angle));
		pos2.setY(pos.x() * sinf(angle) + pos.y() * cosf(angle));
		RoadVertex* v = new RoadVertex(pos2);
		RoadVertexDesc desc = boost::add_vertex(roads->graph);
		roads->graph[desc] = v;
	}
	for (int i = 0; i < num - 2; i++) {
		QVector2D pos = orig + QVector2D(i * length + length, size);
		QVector2D pos2;
		pos2.setX(pos.x() * cosf(angle) - pos.y() * sinf(angle));
		pos2.setY(pos.x() * sinf(angle) + pos.y() * cosf(angle));
		RoadVertex* v = new RoadVertex(pos2);
		RoadVertexDesc desc = boost::add_vertex(roads->graph);
		roads->graph[desc] = v;
	}

	// エッジを作成
	for (int i = 0; i < num - 2; i++) {
		for (int j = 0; j < num - 1; j++) {
			RoadEdge* e = new RoadEdge(2, 2, false);
			QVector2D pos = orig + QVector2D(j * length, i * length + length);
			for (int k = 0; k <= 10; k++) {
				QVector2D pos2 = pos + QVector2D((float)k * 0.1f * length, length * 0.1f * sinf((float)k * M_PI * 2 * 0.1f));
				QVector2D pos3;
				pos3.setX(pos2.x() * cosf(angle) - pos2.y() * sinf(angle));
				pos3.setY(pos2.x() * sinf(angle) + pos2.y() * cosf(angle));
				e->addPoint(pos3);
			}
			
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(i * num + j, i * num + j + 1, roads->graph);
			roads->graph[edge_pair.first] = e;
		}
	}
	for (int i = 0; i < num - 3; i++) {
		for (int j = 0; j < num - 2; j++) {
			RoadEdge* e = new RoadEdge(2, 2, false);
			QVector2D pos = orig + QVector2D(j * length + length, i * length + length);
			for (int k = 0; k <= 10; k++) {
				QVector2D pos2 = pos + QVector2D(length * 0.1f * sinf((float)k * M_PI * 2 * 0.1f), (float)k * 0.1f * length);
				QVector2D pos3;
				pos3.setX(pos2.x() * cosf(angle) - pos2.y() * sinf(angle));
				pos3.setY(pos2.x() * sinf(angle) + pos2.y() * cosf(angle));
				e->addPoint(pos3);
			}
			
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(i * num + 1 + j, i * num + 1 + j + num, roads->graph);
			roads->graph[edge_pair.first] = e;

			//addEdge(roads, i * num + 1 + j, i * num + 1 + j + num, 2, 2);
		}
	}
	for (int i = 0; i < num - 2; i++) {
		RoadEdge* e = new RoadEdge(2, 2, false);
		QVector2D pos = orig + QVector2D(i * length + length, 0);
		for (int k = 0; k <= 10; k++) {
			QVector2D pos2 = pos + QVector2D(length * 0.1f * sinf((float)k * M_PI * 2 * 0.1f), (float)k * 0.1f * length);
			QVector2D pos3;
			pos3.setX(pos2.x() * cosf(angle) - pos2.y() * sinf(angle));
			pos3.setY(pos2.x() * sinf(angle) + pos2.y() * cosf(angle));
			e->addPoint(pos3);
		}
			
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(num * (num - 2) + i, i + 1, roads->graph);
		roads->graph[edge_pair.first] = e;

		//addEdge(roads, num * (num - 2) + i, i + 1, 2, 2);
	}
	for (int i = 0; i < num - 2; i++) {
		RoadEdge* e = new RoadEdge(2, 2, false);
		QVector2D pos = orig + QVector2D(i * length + length, size - length);
		for (int k = 0; k <= 10; k++) {
			QVector2D pos2 = pos + QVector2D(length * 0.1f * sinf((float)k * M_PI * 2 * 0.1f), (float)k * 0.1f * length);
			QVector2D pos3;
			pos3.setX(pos2.x() * cosf(angle) - pos2.y() * sinf(angle));
			pos3.setY(pos2.x() * sinf(angle) + pos2.y() * cosf(angle));
			e->addPoint(pos3);
		}
			
		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(num * (num - 2) + (num - 2) + i, num * (num - 3) + i + 1, roads->graph);
		roads->graph[edge_pair.first] = e;
		
		//addEdge(roads, num * (num - 2) + (num - 2) + i, num * (num - 3) + i + 1, 2, 2);
	}

	return roads;
}

/**
 * 放射線状の道路網を作成する。
 *
 * @param size		一辺の長さ [m]
 * @param num		円を何個作るか？
 */
RoadGraph* GraphUtil::createRadialNetwork(float size, int num) {
	RoadGraph* roads = new RoadGraph();

	float length = size / (float)(num + 1) / 2.0f;

	// 頂点を追加
	RoadVertex* v = new RoadVertex(QVector2D(0, 0));
	RoadVertexDesc desc = boost::add_vertex(roads->graph);
	roads->graph[desc] = v;

	for (int i = 0; i < num + 1; i++) {
		for (int j = 0; j < 12; j++) {
			float theta = (float)j / 12.0f * M_PI * 2.0f;
			RoadVertex* v = new RoadVertex(length * QVector2D((float)(i + 1) * cosf(theta), (float)(i + 1) * sinf(theta)));
			RoadVertexDesc desc = boost::add_vertex(roads->graph);
			roads->graph[desc] = v;
		}
	}

	// エッジを追加
	for (int i = 0; i < 12; i++) {
		addEdge(roads, 0, i + 1, 2, 2);
	}
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < 12; j++) {
			addEdge(roads, 1 + i * 12 + j, 1 + (i + 1) * 12 + j, 2, 2);
		}
	}
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < 12; j++) {
			float theta = (float)j / 12.0f * M_PI * 2.0f;
			float dt = 1.0f / 12.0f * M_PI * 2.0f;
			RoadEdge* e = new RoadEdge(2, 2, false);
			for (int k = 0; k <= 4; k++) {
				QVector2D pos = length * QVector2D((float)(i + 1) * cosf(theta + dt * (float)k / 4.0f), (float)(i + 1) * sinf(theta + dt * (float)k / 4.0f));
				e->addPoint(pos);
			}
			
			std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(1 + i * 12 + j, 1 + i * 12 + (j + 1) % 12, roads->graph);
			roads->graph[edge_pair.first] = e;
			
			//addEdge(roads, 1 + i * 12 + j, 1 + i * 12 + (j + 1) % 12, 2, 2);
		}
	}

	return roads;
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

	qDebug() << "Degrees:";
	for (int i = 0; i < 10; i++) {
		qDebug() << i << ": " << degreesHistogram[i];
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

	qDebug() << "Lanes:";
	for (int i = 0; i < 10; i++) {
		qDebug() << i << ": " << lanesHistogram[i];
	}
}
