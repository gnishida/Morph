#pragma once

#include "RoadGraph.h"
#include <vector>

class GraphUtil {
protected:
	GraphUtil() {}

public:
	static float getTotalEdgeLength(RoadGraph* roads, RoadVertexDesc v);
	static int getNumVertices(RoadGraph* roads, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph* roads, int index, bool onlyValidVertex = true);
	static int getVertexIndex(RoadGraph* roads, RoadVertexDesc desc, bool onlyValidVertex = true);
	static RoadVertexDesc copyVertex(RoadGraph* roads, RoadVertexDesc v, bool virtFlag = true);
	static void moveVertex(RoadGraph* roads, RoadVertexDesc v, QVector2D pt);
	static void collapseVertex(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static void collapseEdge(RoadGraph* roads, RoadEdgeDesc e);
	static int getNumEdges(RoadGraph* roads, bool onlyValidEdge = true);
	static RoadEdgeDesc addEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, unsigned int lanes, unsigned int type, bool oneWay = false);
	static std::vector<RoadVertexDesc> getNeighbors(RoadGraph* roads, RoadVertexDesc v);
	static bool isNeighbor(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static bool isReachable(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt);
	static bool hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static bool isDirectlyConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static RoadEdgeDesc getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, bool onlyValidEdge = true);
	static std::vector<QVector2D> getOrderedPolyLine(RoadGraph* roads, RoadEdgeDesc e);
	static int getDegree(RoadGraph* roads, RoadVertexDesc v, bool onlyValidEdge = true);
	static RoadVertexDesc findNearestVertex(RoadGraph* roads, const QVector2D &pt);
	static RoadVertexDesc findNearestVertex(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc ignore);
	static RoadVertexDesc findConnectedNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc v);
	static RoadEdgeDesc findNearestEdge(RoadGraph* roads, const QVector2D &pt, float& dist, QVector2D& closestPt, bool onlyValidEdge = true);
	static RoadEdgeDesc findNearestEdge(RoadGraph* roads, RoadVertexDesc v, float& dist, QVector2D& closestPt, bool onlyValidEdge = true);
	static std::vector<RoadVertexDesc> getChildren(RoadGraph* roads, RoadVertexDesc v);
	static void simplify(RoadGraph* roads, float dist_threshold);
	static void normalize(RoadGraph* roads);
	static void singlify(RoadGraph* roads);
	static void planarify(RoadGraph* roads);
	static RoadGraph* copyRoads(RoadGraph* roads);
	static float computeMinDiff(std::vector<float> *data1, std::vector<float> *data2);
	static float normalizeAngle(float angle);
	static float diffAngle(QVector2D& dir1, QVector2D& dir2);
	static float diffAngle(float angle1, float angle2);


	static float computeMinDiffInTopology(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2);
	static float computeDiffInTopology(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc> correspondence1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc> correspondence2);
	static bool nextSequence(std::vector<int>& seq, int N);

	static std::vector<RoadEdgeDesc> getMajorEdges(RoadGraph* roads, int num);

	static RoadGraph* createGridNetwork(float size, int num);
	static RoadGraph* createCurvyNetwork(float size, int num, float angle);
	static RoadGraph* createRadialNetwork(float size, int num);

	static void printStatistics(RoadGraph* roads);
};

