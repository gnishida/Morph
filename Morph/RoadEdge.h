#pragma once

#include <qvector2d.h>
#include <vector>

class RoadEdge {
public:
	unsigned int lanes;
	unsigned int type;
	bool oneWay;
	std::vector<QVector2D> polyLine;
	float weight;
	float importance;

	bool valid;			// このエッジが、有効か、無効か
	bool seed;			// このエッジは、フォレスト構築の際に、シードとして使用されたということ
	int group;			// このエッジが、フォレスト構築の際に、どのツリーに属したかということ
	bool fullyPaired;	// このエッジは、完全マッチングにおいて、対応相手が見つかったということ

public:
	RoadEdge(unsigned int lanes, unsigned int type, bool oneWay);
	~RoadEdge();
	
	int getNumLanes();
	float getLength();
	int getType();
	std::vector<QVector2D> getPolyLine();

	void addPoint(const QVector2D &pt);
	float getWidth();
};

