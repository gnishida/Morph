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
};

