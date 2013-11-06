#pragma once

#include "RoadGraph.h"
#include <vector>

class GraphUtil {
protected:
	GraphUtil() {}

public:
	static float getTotalEdgeLength(RoadGraph* roads, RoadVertexDesc v);
	static void removeVertex(RoadGraph* roads, RoadVertexDesc v);
	static void removeEdge(RoadGraph* roads, RoadEdgeDesc e);
	static std::vector<RoadVertexDesc> getNeighbors(RoadGraph* roads, RoadVertexDesc v);
	static bool isNeighbor(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static bool isReachable(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt);
	static bool hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static RoadEdgeDesc getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, bool onlyValidEdge = true);
	static int getDegree(RoadGraph* roads, RoadVertexDesc v, bool onlyValidEdge = true);
	static RoadVertexDesc findNearestNeighbor(RoadGraph* roads, const QVector2D &pt);
	static RoadVertexDesc findNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc ignore);
	static std::vector<RoadVertexDesc> getChildren(RoadGraph* roads, RoadVertexDesc v);
};

