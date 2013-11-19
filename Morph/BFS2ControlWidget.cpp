#include "BFS2ControlWidget.h"
#include "Morph.h"

BFS2ControlWidget::BFS2ControlWidget(Morph* parent) : QDockWidget("BFS2 Control Widget", (QWidget*)parent) {
	ui.setupUi(this);
	this->parent = parent;
	this->bfs = NULL;

	// setup the UI
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(moveSequence(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevSequence()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextSequence()));

	// initialize BFS
	bfs = new BFS2("paris_4000.gsm", "london_4000.gsm");
	bfs->buildTree();
	ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
	ui.horizontalSlider->setValue(0);
	parent->update();
}

void BFS2ControlWidget::draw(QPainter* painter, int offset, float scale) {
	if (bfs == NULL) return;

    bfs->draw(painter, offset, scale);
}

void BFS2ControlWidget::moveSequence(int value) {
	if (bfs == NULL) return;
	bfs->selectSequence(value);
	parent->update();
}

void BFS2ControlWidget::prevSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	parent->update();
}

void BFS2ControlWidget::nextSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	parent->update();
}