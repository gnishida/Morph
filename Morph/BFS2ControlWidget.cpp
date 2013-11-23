#include "BFS2ControlWidget.h"
#include "Morph.h"
#include <qfiledialog.h>

BFS2ControlWidget::BFS2ControlWidget(Morph* parent) : ControlWidget("BFS2 Control Widget", parent) {
	ui.setupUi(this);
	this->bfs = NULL;

	// setup the UI
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.pushButtonLoadRoad1, SIGNAL(clicked()), this, SLOT(loadRoad1()));
	connect(ui.pushButtonLoadRoad2, SIGNAL(clicked()), this, SLOT(loadRoad2()));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(moveSequence(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevSequence()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextSequence()));

	// initialize BFS
	bfs = new BFS2();
}

void BFS2ControlWidget::draw(QPainter* painter) {
	if (bfs == NULL) return;

    bfs->draw(painter);
}

void BFS2ControlWidget::selectVertex(float x, float y) {
}

void BFS2ControlWidget::loadRoad1() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		bfs->setRoad1(filename.toUtf8().data());

		ui.lineEditRoad1->setText(filename.split("/").last().split(".").at(0));

		if (bfs->sequence.size() > 0) {
			ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
			ui.horizontalSlider->setValue(0);
			update();
		}
	}
}

void BFS2ControlWidget::loadRoad2() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		bfs->setRoad2(filename.toUtf8().data());

		ui.lineEditRoad2->setText(filename.split("/").last().split(".").at(0));

		if (bfs->sequence.size() > 0) {
			ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
			ui.horizontalSlider->setValue(0);
			update();
		}
	}
}

void BFS2ControlWidget::moveSequence(int value) {
	if (bfs == NULL) return;
	bfs->selectSequence(value);
	update();
}

void BFS2ControlWidget::prevSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	update();
}

void BFS2ControlWidget::nextSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	update();
}