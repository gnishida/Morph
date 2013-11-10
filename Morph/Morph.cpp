#include "Morph.h"
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
	connect(ui.actionMTT, SIGNAL(triggered()), this, SLOT(startMTT()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	width = height = 2000;
	//width = height = 10000;
	//cellLength = 1000;

	morphing = NULL;
	morphing2 = NULL;
	bfs = NULL;
	mtt = NULL;

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

	if (mode == 3 && bfs != NULL) {
		bfs->draw(&painter, t, width / 2 + 150, 800.0f / width);
	}

	if (mode == 4 && mtt != NULL) {
		mtt->draw(&painter, t, width / 2 + 150, 800.0f / width);
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

	if (bfs == NULL) {
		bfs = new BFS(this, "roads1.gsm", "roads2.gsm");
		bfs->buildTree();
	}

	t = 1.0f;
	timer->start(100);
}

void Morph::startMTT() {
	timer->stop();

	mode = 4;


	if (mtt == NULL) {
		mtt = new MTT(this, "roads1.gsm", "roads2.gsm");
		mtt->buildTree();
	}

	// DocWidgetの表示
	widgetMTT = new MTTControlWidget(this, mtt);
	addDockWidget(Qt::LeftDockWidgetArea, widgetMTT);

}

void Morph::tick() {
	update();

	t -= 0.02f;
	if (t < 0.0f) {
		t = 0.0f;
		//timer->stop();
	}
}

