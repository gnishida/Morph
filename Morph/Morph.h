#ifndef MORPH_H
#define MORPH_H

#include <QtGui/QMainWindow>
#include "RoadGraph.h"
#include "BBox.h"
#include "Morphing.h"
#include "ui_Morph.h"
#include <qmap.h>
#include <qtimer.h>

class Morph : public QMainWindow {
	Q_OBJECT

public:
	QTimer* timer;
	float t;

	int width;
	int height;
	//int cellLength;

	Morphing* morphing;

	// roads
	RoadGraph* roadsA;
	//RoadGraph* roadsB;
	//RoadGraph* interpolated_roads;

	// neighbor
	//QMap<RoadVertexDesc, RoadVertexDesc> neighbor1;
	//QMap<RoadVertexDesc, RoadVertexDesc> neighbor2;

public:
	Morph(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Morph();

protected:
    void paintEvent(QPaintEvent *);

private:
	Ui::MorphClass ui;

private slots:
	void start();
	void tick();

public:
	
	void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2);
	
};


#endif // MORPH_H
