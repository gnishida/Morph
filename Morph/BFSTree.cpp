#include "BFSTree.h"
#include "GraphUtil.h"

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
std::vector<RoadVertexDesc>& BFSTree::getChildren(RoadVertexDesc node) {
	if (!children.contains(node)) {
		std::vector<RoadVertexDesc> c;
		children[node] = c;
	}

	return children[node];
}

void BFSTree::addChild(RoadVertexDesc parent, RoadVertexDesc child) {
	std::vector<RoadVertexDesc> list = getChildren(parent);
	list.push_back(child);
	children[parent] = list;
}

RoadVertexDesc BFSTree::getParent(RoadVertexDesc node) {
	for (QMap<RoadVertexDesc, std::vector<RoadVertexDesc> >::iterator it = children.begin(); it != children.end(); ++it) {
		RoadVertexDesc parent = it.key();
		for (int i = 0; i < children[parent].size(); i++) {
			if (children[parent][i] == node) return parent;
		}
	}

	throw "No specified node found.";
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

	// node2の子供を作成する
	std::vector<RoadVertexDesc> c = children[node2];
	c.push_back(v_desc);
	children[node2] = c;

	for (int i = 0; i < children[node1].size(); i++) {
		RoadVertexDesc c_desc = copySubTree(children[node1][i], v_desc);

		// エッジを取得
		RoadEdgeDesc e_desc = GraphUtil::getEdge(roads, node1, children[node1][i]);

		// エッジを作成する
		RoadEdge* new_e = new RoadEdge(roads->graph[e_desc]->lanes, roads->graph[e_desc]->type);
		new_e->addPoint(roads->graph[node1]->getPt());
		new_e->addPoint(roads->graph[children[node1][i]]->getPt());

		std::pair<RoadEdgeDesc, bool> edge_pair = boost::add_edge(node2, c_desc, roads->graph);
		roads->graph[edge_pair.first] = new_e;
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
			if (!roads->graph[child]->valid) continue;
			try {
				if (child == getParent(parent)) continue;
			} catch (const char* ex) {
			}

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
