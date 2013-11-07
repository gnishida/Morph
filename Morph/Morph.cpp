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

	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));
	connect(timer, SIGNAL(timeout()), this, SLOT(tick()) );

	width = height = 2000;
	//width = height = 10000;
	//cellLength = 1000;

	morphing = NULL;
	morphing2 = NULL;
	mmt = NULL;
}

Morph::~Morph() {
}

void Morph::paintEvent(QPaintEvent *) {
    QPainter painter(this);

	if (morphing != NULL) {
		morphing->draw(&painter, t, width / 2 + 150, 800.0f / width);
	}
	
	if (morphing2 != NULL) {
		morphing2->draw(&painter, t, width / 2 + 150, 800.0f / width);
	}

	if (mmt != NULL) {
		mmt->draw(&painter, width / 2 + 150, 800.0f / width);
	}
}

void Morph::start() {
	timer->stop();

	if (morphing == NULL) {
		//morphing = new Morphing(this);
		//morphing->initRoads("roads1.gsm", "roads2.gsm");
	}

	if (morphing2 == NULL) {
		morphing2 = new Morphing2(this);
		morphing2->initRoads("roads1.gsm", "roads2.gsm");
	}

	if (mmt == NULL) {
		//mmt = new MMT(this, "roads1.gsm");
		//mmt->buildTree();
	}

	t = 1.0f;

	timer->start(100);
}

void Morph::tick() {
	update();

	t -= 0.02f;
	if (t < 0.0f) {
		t = 0.0f;
		//timer->stop();
	}
}

