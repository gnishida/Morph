﻿#include "Morph.h"
#include <qset.h>
#include <qpainter.h>
#include <qcolor.h>
#include <limits>
#include <time.h>
#include <qdebug.h>
#include <boost/graph/graph_utility.hpp>

Morph::Morph(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	timer = new QTimer(this);

	connect(ui.actionNearestNeighbor, SIGNAL(triggered()), this, SLOT(startNearestNeighbor()));
	connect(ui.actionNearestNeighborConnectivity, SIGNAL(triggered()), this, SLOT(startNearestNeighborConnectivity()));
	connect(ui.actionBFS, SIGNAL(triggered()), this, SLOT(startBFS()));
	connect(ui.actionBFS2, SIGNAL(triggered()), this, SLOT(startBFS2()));
	connect(ui.actionBFSProp, SIGNAL(triggered()), this, SLOT(startBFSProp()));
	connect(ui.actionMTT, SIGNAL(triggered()), this, SLOT(startMTT()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	width = height = 2000;
	//width = height = 10000;
	//cellLength = 1000;

	morphing = NULL;
	morphing2 = NULL;

	widgetBFS = new BFSControlWidget(this);
	widgetBFS->hide();

	widgetBFS2 = new BFS2ControlWidget(this);
	widgetBFS2->hide();

	widgetBFSProp = new BFSPropControlWidget(this);
	widgetBFSProp->hide();

	widgetMTT = new MTTControlWidget(this);
	widgetMTT->hide();

	mode = 0;
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	if (mode == 1 && morphing != NULL) {
		morphing->draw(&painter, t, width / 2 + 150, 800.0f / width);
	}
	
	if (mode == 2 && morphing2 != NULL) {
		morphing2->draw(&painter, t, width / 2 + 150, 800.0f / width);
	}

	if (mode == 3) {
		widgetBFS->draw(&painter, width / 2 + 150, 800.0f / width);
	}

	if (mode == 4) {
		widgetBFS2->draw(&painter, width / 2 + 150, 800.0f / width);
	}

	if (mode == 5) {
		widgetBFSProp->draw(&painter, width / 2 + 150, 800.0f / width);
	}

	if (mode == 6) {
		widgetMTT->draw(&painter, width / 2 + 150, 800.0f / width);
	}
}

void Morph::startNearestNeighbor() {
	timer->stop();

	mode = 1;

	if (morphing == NULL) {
		morphing = new Morphing(this);
		morphing->initRoads("roads1.gsm", "roads2.gsm");
	}

	t = 1.0f;
	timer->start(100);
}

void Morph::startNearestNeighborConnectivity() {
	timer->stop();

	mode = 2;

	if (morphing2 == NULL) {
		morphing2 = new Morphing2(this);
		morphing2->initRoads("roads1.gsm", "roads2.gsm");
	}

	t = 1.0f;
	timer->start(100);
}

void Morph::startBFS() {
	timer->stop();

	mode = 3;

	// DocWidgetの表示
	widgetBFS->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFS);

	widgetBFS2->hide();
	widgetBFSProp->hide();
	widgetMTT->hide();
}

void Morph::startBFS2() {
	timer->stop();

	mode = 4;

	// DocWidgetの表示
	widgetBFS2->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFS2);

	widgetBFS->hide();
	widgetBFSProp->hide();
	widgetMTT->hide();
}

void Morph::startBFSProp() {
	timer->stop();

	mode = 5;

	// DocWidgetの表示
	widgetBFSProp->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFSProp);

	widgetBFS->hide();
	widgetBFS2->hide();
	widgetMTT->hide();
}

void Morph::startMTT() {
	timer->stop();

	mode = 6;

	// DocWidgetの表示
	widgetMTT->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetMTT);

	widgetBFS->hide();
	widgetBFS2->hide();
	widgetBFSProp->hide();
}

void Morph::tick() {
	update();

	t -= 0.02f;
	if (t < 0.0f) {
		t = 0.0f;
		//timer->stop();
	}
}

