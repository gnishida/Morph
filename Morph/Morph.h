#ifndef MORPH_H
#define MORPH_H

#include <QtGui/QMainWindow>
#include "RoadGraph.h"
#include "BBox.h"
#include "Morphing.h"
#include "Morphing2.h"
#include "BFS.h"
#include "MTT.h"
#include "ui_Morph.h"
#include "BFSControlWidget.h"
#include "BFS2ControlWidget.h"
#include "BFSPropControlWidget.h"
#include "MTTControlWidget.h"
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
	//BFS* bfs;

	BFSControlWidget* widgetBFS;
	BFS2ControlWidget* widgetBFS2;
	BFSPropControlWidget* widgetBFSProp;
	MTTControlWidget* widgetMTT;

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
	void startBFS();
	void startBFS2();
	void startBFSProp();
	void startMTT();
	void tick();

public:
		
};


#endif // MORPH_H
