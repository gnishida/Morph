#include "BFSForest.h"
#include "GraphUtil.h"

/**
 * コンストラクタ
 * rootsには、２つの頂点を１ペアとして、頂点ペアが並んでいる。各ペアは、エッジに対応する。
 * つまり、roots.size()は、偶数であるはず。
 */
BFSForest::BFSForest(RoadGraph* roads, QList<RoadVertexDesc> roots) {
	this->roads = roads;
	this->roots = roots;

	buildForest();
}

BFSForest::~BFSForest() {
}

/**
 * 子ノードのリストを返却する。
 */
std::vector<RoadVertexDesc>& BFSForest::getChildren(RoadVertexDesc node) {
	if (!children.contains(node)) {
		std::vector<RoadVertexDesc> c;
		children[node] = c;
	}

	return children[node];
}

/**
 * 子ノードを追加する。
 */
void BFSForest::addChild(RoadVertexDesc parent, RoadVertexDesc child) {
	std::vector<RoadVertexDesc> list = getChildren(parent);
	list.push_back(child);
	children[parent] = list;
}

/**
 * 親ノードのリストを返却する。
 */
QList<RoadVertexDesc> BFSForest::getParent(RoadVertexDesc node) {
	QList<RoadVertexDesc> ret;

	for (QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >::iterator it = children.begin(); it != children.end(); ++it) {
		RoadVertexDesc parent = it.key();
		for (int i = 0; i < children[parent].size(); i++) {
			if (children[parent][i] == node) {
				ret.push_back(parent);
			}
		}
	}

	return ret;
}

/**
 * ルートノードを返却する。
 */
QList<RoadVertexDesc> BFSForest::getRoots() {
	return roots;
}

void BFSForest::buildForest() {
	QList<RoadVertexDesc> seeds;
	QList<int> groups;

	QMap<RoadEdgeDesc, bool> visitedEdge;
	QMap<RoadVertexDesc, bool> visitedVertex;

	// ルートとして与えられた頂点について
	for (int i = 0; i < roots.size() / 2; i++) {
		RoadVertexDesc src = roots[i * 2];
		RoadVertexDesc tgt = roots[i * 2 + 1];

		// エッジの取得
		RoadEdgeDesc e_desc = GraphUtil::getEdge(roads, src, tgt);

		// エッジのグループ、seedフラグを設定
		roads->graph[e_desc]->group = i;
		roads->graph[e_desc]->seed = true;

		// 頂点srcが既存のシードと重複している場合
		if (seeds.contains(src)) {
			// 頂点srcをコピーする
			RoadVertex* v = new RoadVertex(roads->graph[src]->pt);
			RoadVertexDesc new_src = boost::add_vertex(roads->graph);
			roads->graph[new_src] = v;

			// 古いエッジを削除
			roads->graph[e_desc]->valid = false;

			// 新しいエッジを追加
			e_desc = GraphUtil::addEdge(roads, new_src, tgt, roads->graph[e_desc]);

			src = new_src;
		}

		// 頂点tgtが既存のシードと重複している場合
		if (seeds.contains(tgt)) {
			// 頂点tgtをコピーする
			RoadVertex* v = new RoadVertex(roads->graph[tgt]->pt);
			RoadVertexDesc new_tgt = boost::add_vertex(roads->graph);
			roads->graph[new_tgt] = v;

			// 古いエッジを削除
			roads->graph[e_desc]->valid = false;

			// 新しいエッジを追加
			e_desc = GraphUtil::addEdge(roads, src, new_tgt, roads->graph[e_desc]);

			tgt = new_tgt;
		}

		// src、tgtが更新されたかも知れないので、おおもとのデータも更新しておく
		roots[i * 2] = src;
		roots[i * 2 + 1] = tgt;

		// シードを登録する
		seeds.push_back(src);
		seeds.push_back(tgt);
		groups.push_back(i);
		groups.push_back(i);

		// ルートエッジ・頂点を訪問済みとマークする
		visitedEdge[e_desc] = true;
		visitedVertex[src] = true;
		visitedVertex[tgt] = true;
	}

	// ルート頂点リストからスタートして、BFSで全頂点を訪問する
	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		int group = groups.front();
		groups.pop_front();

		std::vector<RoadVertexDesc> children;

		// 隣接ノードリストを先に洗い出す
		std::vector<RoadVertexDesc> nodes;
		std::vector<RoadEdgeDesc> edges;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(parent, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;
			if (visitedEdge[*ei]) continue;

			// 隣接ノードを取得
			RoadVertexDesc child = boost::target(*ei, roads->graph);
			if (!roads->graph[child]->valid) continue;

			if (getParent(parent).contains(child)) continue;

			nodes.push_back(child);
			edges.push_back(*ei);

			// 当該エッジを通過したとマークする
			visitedEdge[*ei] = true;
		}

		// 洗い出した隣接ノードに対して、訪問する
		for (int i = 0; i < nodes.size(); i++) {
			RoadVertexDesc child = nodes[i];

			if (visitedVertex.contains(child)) { // 訪問済みの場合
				RoadEdgeDesc orig_e_desc = GraphUtil::getEdge(roads, parent, child);

				// もともとのエッジを無効にする
				roads->graph[orig_e_desc]->valid = false;

				// 対象ノードが訪問済みの場合、対象ノードをコピーして子ノードにする
				RoadVertexDesc child2 = GraphUtil::addVertex(roads, roads->graph[child]);
				roads->graph[child2]->virt = false;

				// エッジ作成
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, parent, child2, roads->graph[orig_e_desc]);

				roads->graph[e_desc]->group = group;

				children.push_back(child2);

				// 分割しない案
				//children.push_back(child);
			} else { // 未訪問の場合
				visitedVertex[child] = true;
				roads->graph[edges[i]]->group = group;

				children.push_back(child);

				seeds.push_back(child);
				groups.push_back(group);
			}
		}

		this->children.insert(parent, children);
	}
}