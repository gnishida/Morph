#pragma once

#include "RoadGraph.h"
#include "BBox.h"
#include <vector>

class GraphUtil {
protected:
	GraphUtil() {}

public:
	// 頂点関係の関数
	static int getNumVertices(RoadGraph* roads, bool onlyValidVertex = true);
	static int getNumConnectedVertices(RoadGraph* roads, RoadVertexDesc start, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph* roads, int index, bool onlyValidVertex = true);
	static bool getVertex(RoadGraph* roads, QVector2D pos, float threshold, RoadVertexDesc& desc, bool onlyValidVertex = true);
	static int getVertexIndex(RoadGraph* roads, RoadVertexDesc desc, bool onlyValidVertex = true);
	static RoadVertexDesc copyVertex(RoadGraph* roads, RoadVertexDesc v, bool virtFlag = true);
	static void moveVertex(RoadGraph* roads, RoadVertexDesc v, QVector2D pt);
	static void collapseVertex(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static int getDegree(RoadGraph* roads, RoadVertexDesc v, bool onlyValidEdge = true);

	// エッジ関係の関数
	static float getTotalEdgeLength(RoadGraph* roads, RoadVertexDesc v);
	static void collapseEdge(RoadGraph* roads, RoadEdgeDesc e);
	static int getNumEdges(RoadGraph* roads, bool onlyValidEdge = true);
	static RoadEdgeDesc addEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, unsigned int lanes, unsigned int type, bool oneWay = false);
	static bool hasEdge(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static RoadEdgeDesc getEdge(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt, bool onlyValidEdge = true);
	static std::vector<QVector2D> getOrderedPolyLine(RoadGraph* roads, RoadEdgeDesc e);
	static void orderPolyLine(RoadGraph* roads, RoadEdgeDesc e, RoadVertexDesc src);
	static void movePolyLine(RoadGraph* roads, RoadEdgeDesc e, QVector2D& src_pos, QVector2D& tgt_pos);
	static std::vector<RoadEdgeDesc> getMajorEdges(RoadGraph* roads, int num);
	static void removeDeadEnd(RoadGraph* roads);

	// 道路網全体に関する関数
	static RoadGraph* copyRoads(RoadGraph* roads);
	static void copyRoads(RoadGraph* roads1, RoadGraph* roads2);
	static BBox getAABoundingBox(RoadGraph* roads);
	static BBox getBoudingBox(RoadGraph* roads, float theta1, float theta2, float theta_step = 0.087f);
	static RoadGraph* extractMajorRoad(RoadGraph* roads, bool remove = true);
	static float extractMajorRoad(RoadGraph* roads, RoadEdgeDesc root, QList<RoadEdgeDesc>& path);

	// 隣接関係、接続性などの関数
	static std::vector<RoadVertexDesc> getNeighbors(RoadGraph* roads, RoadVertexDesc v, bool onlyValidVertex = true);
	static bool isNeighbor(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2);
	//static bool isReachable(RoadGraph* roads, RoadVertexDesc src, RoadVertexDesc tgt);
	static bool isDirectlyConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static bool isConnected(RoadGraph* roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static RoadVertexDesc findNearestVertex(RoadGraph* roads, const QVector2D &pt);
	static RoadVertexDesc findNearestVertex(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc ignore);
	static RoadVertexDesc findConnectedNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc v);
	static RoadEdgeDesc findNearestEdge(RoadGraph* roads, const QVector2D &pt, float& dist, QVector2D& closestPt, bool onlyValidEdge = true);
	static RoadEdgeDesc findNearestEdge(RoadGraph* roads, RoadVertexDesc v, float& dist, QVector2D& closestPt, bool onlyValidEdge = true);
	static std::vector<RoadVertexDesc> getChildren(RoadGraph* roads, RoadVertexDesc v);

	// 道路網の変更関係の関数
	static RoadGraph* clean(RoadGraph* roads);
	static void reduce(RoadGraph* roads);
	static bool reduce(RoadGraph* roads, RoadVertexDesc desc);
	static void simplify(RoadGraph* roads, float dist_threshold);
	static void normalize(RoadGraph* roads);
	static void singlify(RoadGraph* roads);
	static void planarify(RoadGraph* roads);
	static void rotate(RoadGraph* roads, float theta);
	static RoadGraph* convertToGridNetwork(RoadGraph* roads, RoadVertexDesc start);
	static RoadGraph* approximateToGridNetwork(RoadGraph* roads, float cellLength, QVector2D orig);
	static void scaleToBBox(RoadGraph* roads, BBox& area);
	static void normalizeBySpring(RoadGraph* roads, BBox& area);
	static bool removeDuplicateEdges(RoadGraph* roads);

	// その他
	static float computeMinDiffAngle(std::vector<float> *data1, std::vector<float> *data2);
	static float normalizeAngle(float angle);
	static float diffAngle(QVector2D& dir1, QVector2D& dir2);
	static float diffAngle(float angle1, float angle2);

	// 道路網の比較関数
	static float computeMinUnsimilarity(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2);
	static float computeUnsimilarity(RoadGraph* roads1, QMap<RoadVertexDesc, RoadVertexDesc>& map1, RoadGraph* roads2, QMap<RoadVertexDesc, RoadVertexDesc>& map2);

	static bool nextSequence(std::vector<int>& seq, int N);

	// 統計情報
	static float computeAvgEdgeLength(RoadGraph* roads);

	// サンプル道路網を生成する関数
	static RoadGraph* createGridNetwork(float size, int num);
	static RoadGraph* createCurvyNetwork(float size, int num, float angle);
	static RoadGraph* createRadialNetwork(float size, int num);

	// デバッグ用の関数
	static void printStatistics(RoadGraph* roads);
};

