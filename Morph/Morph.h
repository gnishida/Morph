#ifndef MORPH_H
#define MORPH_H

#include <QtGui/QMainWindow>
#include "RoadGraph.h"
#include "BBox.h"
#include "Morphing.h"
#include "Morphing2.h"
#include "MMT.h"
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
	Morphing2* morphing2;
	MMT* mmt;

	int mode;

public:
	Morph(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Morph();

protected:
    void paintEvent(QPaintEvent *);

private:
	Ui::MorphClass ui;

private slots:
	void startNearestNeighbor();
	void startNearestNeighborConnectivity();
	void startMMT();
	void tick();

public:
	
	//void drawGraph(QPainter *painter, RoadGraph *roads, QColor col, int offset, float scale);
	//void drawRelation(QPainter *painter, RoadGraph *roads1, QMap<RoadVertexDesc, RoadVertexDesc> neighbor1, RoadGraph *roads2, QMap<RoadVertexDesc, RoadVertexDesc> neighbor2);
	
};


#endif // MORPH_H
