#include "Morph.h"
#include <qset.h>
#include <qpainter.h>
#include <qcolor.h>
#include <limits>
#include <time.h>
#include <qdebug.h>
#include <qscrollarea.h>
#include <boost/graph/graph_utility.hpp>

Morph::Morph(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	canvas = new Canvas(this);
	
	scrollArea = new QScrollArea();
	scrollArea->setWidget(canvas);
	scrollArea->setWidgetResizable(true);
	setCentralWidget(scrollArea);

	connect(ui.actionBFS, SIGNAL(triggered()), this, SLOT(startBFS()));
	connect(ui.actionBFS2, SIGNAL(triggered()), this, SLOT(startBFS2()));
	connect(ui.actionBFSMulti, SIGNAL(triggered()), this, SLOT(startBFSMulti()));
	connect(ui.actionBFSProp, SIGNAL(triggered()), this, SLOT(startBFSProp()));
	connect(ui.actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
	connect(ui.actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));

	widgetBFS = new BFSControlWidget(this);
	widgetBFS->hide();

	widgetBFS2 = new BFS2ControlWidget(this);
	widgetBFS2->hide();

	widgetBFSMulti = new BFSMultiControlWidget(this);
	widgetBFSMulti->hide();

	widgetBFSProp = new BFSPropControlWidget(this);
	widgetBFSProp->hide();
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	canvas->update();
}

void Morph::startBFS() {
	canvas->setControlWidget(widgetBFS);

	// DocWidgetの表示
	widgetBFS->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFS);

	widgetBFS2->hide();
	widgetBFSMulti->hide();
	widgetBFSProp->hide();
}

void Morph::startBFS2() {
	canvas->setControlWidget(widgetBFS2);

	// DocWidgetの表示
	widgetBFS2->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFS2);

	widgetBFS->hide();
	widgetBFSProp->hide();
	widgetBFSMulti->hide();
}

void Morph::startBFSMulti() {
	canvas->setControlWidget(widgetBFSMulti);

	// DocWidgetの表示
	widgetBFSMulti->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFSMulti);

	widgetBFS->hide();
	widgetBFS2->hide();
	widgetBFSProp->hide();
}

void Morph::startBFSProp() {
	canvas->setControlWidget(widgetBFSProp);

	// DocWidgetの表示
	widgetBFSProp->show();
	addDockWidget(Qt::RightDockWidgetArea, widgetBFSProp);

	widgetBFS->hide();
	widgetBFS2->hide();
	widgetBFSMulti->hide();
}

void Morph::zoomIn() {
	canvas->zoom(0.2f);
	scrollArea->update();
}

void Morph::zoomOut(){
	canvas->zoom(-0.2f);
	scrollArea->update();
}