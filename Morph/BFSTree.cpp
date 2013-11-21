#include "BFSTree.h"
#include "GraphUtil.h"

BFSTree::BFSTree() {
	this->roads = NULL;
}

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
 * node1_parentの子node1を、node2の子供の一人としてコピーする。
 * コピーされたノードは、virtフラグをtrueにセットする。
 */
RoadVertexDesc BFSTree::copySubTree(RoadVertexDesc node1_parent, RoadVertexDesc node1, RoadVertexDesc node2) {
	// トップノードをコピーする
	RoadVertexDesc v_desc = GraphUtil::copyVertex(roads, node1, true);

	RoadEdgeDesc e_desc = GraphUtil::getEdge(roads, node1_parent, node1);

	// トップノードのために、エッジを作成する
	GraphUtil::addEdge(roads, node2, v_desc, roads->graph[e_desc]->lanes, roads->graph[e_desc]->type, roads->graph[e_desc]->oneWay);

	// 親子関係を登録する
	addChild(node2, v_desc);

	// キューを初期化
	std::list<RoadVertexDesc> seeds1;
	std::list<RoadVertexDesc> seeds2;
	seeds1.push_back(node1);
	seeds2.push_back(v_desc);

	while (!seeds1.empty()) {
		RoadVertexDesc parent1 = seeds1.front();
		seeds1.pop_front();
		RoadVertexDesc parent2 = seeds2.front();
		seeds2.pop_front();

		for (int i = 0; i < children[parent1].size(); i++) {
			// コピー先にノードを作成する
			RoadVertexDesc v_desc = GraphUtil::copyVertex(roads, children[parent1][i], true);

			// エッジを取得
			RoadEdgeDesc e_desc = GraphUtil::getEdge(roads, parent1, children[parent1][i]);

			// コピー先にエッジを作成する
			GraphUtil::addEdge(roads, parent2, v_desc, roads->graph[e_desc]->lanes, roads->graph[e_desc]->type, roads->graph[e_desc]->oneWay);

			seeds1.push_back(children[parent1][i]);
			seeds2.push_back(v_desc);

			// 親子関係を登録する
			addChild(parent2, v_desc);
		}
	}

	return v_desc;
}

/**
 * 指定された頂点以下のサブツリーを削除する。
 */
void BFSTree::removeSubTree(RoadVertexDesc node1) {
	// キューを初期化
	std::list<RoadVertexDesc> seeds;
	seeds.push_back(node1);

	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		// 各子ノードを訪問しながら削除していく
		for (int i = 0; i < children[parent].size(); i++) {
			// エッジを無効にする
			RoadEdgeDesc e_desc = GraphUtil::getEdge(roads, parent, children[parent][i]);
			roads->graph[e_desc]->valid = false;

			// 子ノードを無効にする
			roads->graph[children[parent][i]]->valid = false;

			seeds.push_back(children[parent][i]);
		}

		// 最後に、親子関係を削除する
		this->children.remove(parent);
	}

	roads->graph[node1]->valid = false;
}

/**
 * 指定したノード配下のノード数を返却する。指定したノードも含める。
 */
int BFSTree::getTreeSize(RoadVertexDesc node) {
	int num = 0;

	// キューを初期化
	std::list<RoadVertexDesc> seeds;
	seeds.push_back(node);

	while (!seeds.empty()) {
		RoadVertexDesc parent = seeds.front();
		seeds.pop_front();

		num++;

		// 各子ノードを訪問しながらカウントしていく
		for (int i = 0; i < children[parent].size(); i++) {
			if (!roads->graph[children[parent][i]]->valid) continue;

			seeds.push_back(children[parent][i]);
		}
	}

	return num;
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
