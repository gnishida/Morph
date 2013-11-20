#include "BFSForest.h"
#include "GraphUtil.h"

BFSForest::BFSForest(RoadGraph* roads, std::vector<RoadVertexDesc> roots) : BFSTree(roads, roots[0]) {
	this->roads = roads;
	this->roots = roots;

	buildForest();
}

BFSForest::~BFSForest() {
}

/**
 * ルートノードを返却する。
 */
std::vector<RoadVertexDesc> BFSForest::getRoots() {
	return roots;
}

void BFSForest::buildForest() {
	std::list<RoadVertexDesc> seeds;
	QMap<RoadVertexDesc, bool> visited;

	for (int i = 0; i < roots.size(); i++) {
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

			try {
				if (child == getParent(parent)) continue;
			} catch (const char* ex) {
			}

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
				RoadVertexDesc child2 = GraphUtil::copyVertex(roads, child, false);

				// エッジ作成
				GraphUtil::addEdge(roads, parent, child2, roads->graph[orig_e_desc]->lanes, roads->graph[orig_e_desc]->type, roads->graph[orig_e_desc]->oneWay);

				children.push_back(child2);
			} else { // 未訪問の場合
				visited[child] = true;

				children.push_back(child);

				seeds.push_back(child);
			}
		}

		this->children.insert(parent, children);
	}
}