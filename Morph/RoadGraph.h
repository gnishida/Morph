#pragma once

#include "RoadVertex.h"
#include "RoadEdge.h"
#include <qmap.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace boost;


typedef adjacency_list<vecS, vecS, undirectedS, RoadVertex*, RoadEdge*> BGLGraph;
typedef graph_traits<BGLGraph>::vertex_descriptor RoadVertexDesc;
typedef graph_traits<BGLGraph>::edge_descriptor RoadEdgeDesc;
typedef graph_traits<BGLGraph>::vertex_iterator RoadVertexIter;
typedef graph_traits<BGLGraph>::edge_iterator RoadEdgeIter;
typedef graph_traits<BGLGraph>::out_edge_iterator RoadOutEdgeIter;
typedef graph_traits<BGLGraph>::in_edge_iterator RoadInEdgeIter;

class CollapseAction {
public:
	RoadVertexDesc childNode;
	RoadVertexDesc parentNode;
	std::vector<RoadEdgeDesc> removedEdges;
	std::vector<RoadEdgeDesc> addedEdges;
};

class RoadGraph {
public:
	BGLGraph graph;
	QMap<RoadVertexDesc, std::vector<RoadVertexDesc> > siblings;

	/** 接続性を保持する */
	QMap<RoadVertexDesc, QList<RoadVertexDesc> > connectivities;

	/** 木構造を作成するため、親子関係を保持する */
	QMap<RoadVertexDesc, RoadVertexDesc> childrenToParent;
	QMap<RoadVertexDesc, std::vector<RoadEdgeDesc> > childrenRemovedEdges;
	QMap<RoadVertexDesc, std::vector<RoadEdgeDesc> > parentAddedEdges;

	/** collapseヒストリ */
	std::vector<CollapseAction> collapseHistory;

public:
	RoadGraph();
	~RoadGraph();

	void clear();
	void load(FILE* fp, int roadType);
	void save(FILE* fp);

	void removeIsolatedVertices();

	void computeConnectivities();
	bool isConnected(RoadVertexDesc desc1, RoadVertexDesc desc2);

	std::vector<RoadEdgeDesc> getMajorEdges(RoadGraph* roads, int num);
	bool lessWeight(const RoadEdgeDesc& left, const RoadEdgeDesc& right);

};

class LessWeight {
private:
	RoadGraph* roads;

public:
	LessWeight(RoadGraph* roads);

	bool operator()(const RoadEdgeDesc& left, const RoadEdgeDesc& right) const;
};