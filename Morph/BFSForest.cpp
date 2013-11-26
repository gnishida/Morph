#include "BFSForest.h"
#include "GraphUtil.h"

BFSForest::BFSForest(RoadGraph* roads, QList<RoadVertexDesc> roots) : BFSTree() {
	this->roads = roads;
	this->roots = roots;

	buildForest();
}

BFSForest::~BFSForest() {
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
	QMap<RoadVertexDesc, bool> visited;

	for (int i = 0; i < roots.size(); i++) {
		if (seeds.contains(roots[i])) continue;

		seeds.push_back(roots[i]);
		visited[roots[i]] = true;
	}

	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		std::vector<RoadVertexDesc> children;

		// 隣接ノードリストを先に洗い出す
		std::vector<RoadVertexDesc> nodes;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(parent, roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;
			RoadVertexDesc child = boost::target(*ei, roads->graph);
			if (!roads->graph[child]->valid) continue;

			if (getParent(parent).contains(child)) continue;

			nodes.push_back(child);
		}

		// 洗い出した隣接ノードに対して、訪問する
		for (int i = 0; i < nodes.size(); i++) {
			RoadVertexDesc child = nodes[i];

			if (visited.contains(child)) { // 訪問済みの場合
				RoadEdgeDesc orig_e_desc = GraphUtil::getEdge(roads, parent, child);

				// もともとのエッジを無効にする
				roads->graph[orig_e_desc]->valid = false;

				// 対象ノードが訪問済みの場合、対象ノードをコピーして子ノードにする
				RoadVertexDesc child2 = GraphUtil::addVertex(roads, roads->graph[child]);
				roads->graph[child2]->virt = false;

				// エッジ作成
				GraphUtil::addEdge(roads, parent, child2, roads->graph[orig_e_desc]);

				children.push_back(child2);

				// 分割しない案
				//children.push_back(child);
			} else { // 未訪問の場合
				visited[child] = true;

				children.push_back(child);

				seeds.push_back(child);
			}
		}

		this->children.insert(parent, children);
	}
}