#include "MMT.h"
#include "GraphUtil.h"
#include "Morph.h"
#include <queue>
#include <QtTest/qtest.h>
#include <qdebug.h>

VertexPriority::VertexPriority() {
	desc = -1;
	priority = 0;
}

VertexPriority::VertexPriority(RoadVertexDesc desc, float priority) {
	this->desc = desc;
	this->priority = priority;
}

MMT::MMT(Morph* morph, const char* filename) {
	this->morph = morph;

	FILE* fp = fopen(filename, "rb");
	roads = new RoadGraph();
	roads->load(fp, 2);
}

void MMT::buildTree() {
	// 頂点の中で、degreeが1のものをcollapseしていく
	collapse(roads);

	// 最も短い道路を
}

/**
 * 頂点を、順番にcollapseしていく。
 * ただし、当該頂点から出るエッジの長さが短いものから、優先的にcollapseしていく。
 */
void MMT::collapse(RoadGraph* roads) {
	qDebug() << "collapse start.";

	while (true) {
		float min_len = std::numeric_limits<float>::max();
		RoadEdgeDesc min_e_desc;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads->graph); ei != eend; ++ei) {
			if (!roads->graph[*ei]->valid) continue;

			float len = roads->graph[*ei]->getLength();
			if (len < min_len) {
				min_len = len;
				min_e_desc = *ei;
			}
		}

		if (min_len == std::numeric_limits<float>::max()) break;

		GraphUtil::removeEdge(roads, min_e_desc);

		// 再描画
		morph->roadsA = roads;
		morph->update();

		qDebug() << "remove edge.";

		// 300ミリ秒待機
		QTest::qWait(300);
	}

	qDebug() << "collapse done.";
}