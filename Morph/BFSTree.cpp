#include "BFSTree.h"


BFSTree::BFSTree(RoadGraph* roads, RoadVertexDesc root) {
	this->roads = roads;
	this->root = root;

	buildTree();
}

BFSTree::~BFSTree() {
}

/**
 * 子ノードのリストを返却する。
 */
std::vector<RoadVertexDesc> BFSTree::getChildren(RoadVertexDesc node) {
	return children[node];
}

/**
 * ルートノードを返却する。
 */
RoadVertexDesc BFSTree::getRoot() {
	return root;
}

int BFSTree::getHeight(RoadVertexDesc node) {
	int max_height = 0;

	for (int i = 0; i < children[node].size(); i++) {
		int height = getHeight(children[node][i]);
		if (height > max_height) {
			max_height = height;
		}
	}

	return max_height + 1;
}

/**
 * node1を、node2の子供の一人としてコピーする。
 */
RoadVertexDesc BFSTree::copySubTree(RoadVertexDesc node1, RoadVertexDesc node2) {
	RoadVertex* v = new RoadVertex(roads->graph[node1]->getPt());
	RoadVertexDesc v_desc = boost::add_vertex(roads->graph);
	roads->graph[v_desc] = v;

	std::vector<RoadVertexDesc> c = children[node2];
	c.push_back(v_desc);
	children[node2] = c;

	for (int i = 0; i < children[node1].size(); i++) {
		copySubTree(children[node1][i], v_desc);
	}

	return v_desc;
}

/**
 * 木構造を作成する。
 */
void BFSTree::buildTree() {
	std::list<RoadVertexDesc> seeds;
	seeds.push_back(root);
	QMap<RoadVertexDesc, bool> visited;
	visited[root] = true;

	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		std::vector<RoadVertexDesc> children;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(parent, roads->graph); ei != eend; ++ei) {
			RoadVertexDesc child = boost::target(*ei, roads->graph);
			if (visited.contains(child)) {
				// 対象ノードが訪問済みの場合、対象ノードをコピーして子ノードにする
				RoadVertex* v = new RoadVertex(roads->graph[child]->getPt());
				RoadVertexDesc child2 = boost::add_vertex(roads->graph);
				roads->graph[child2] = v;

				children.push_back(child2);
			} else {
				visited[child] = true;

				children.push_back(child);

				seeds.push_back(child);
			}
		}

		this->children.insert(parent, children);
	}
}
