#include "MTTControlWidget.h"
#include "Morph.h"
#include <qfiledialog.h>

MTTControlWidget::MTTControlWidget(Morph* parent) : QDockWidget("MTT Control Widget", (QWidget*)parent) {
	ui.setupUi(this);
	this->parent = parent;
	this->mtt = NULL;

	// setup the UI
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateRoads(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevRoads()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextRoads()));
	connect(ui.pushButtonLoadGSM, SIGNAL(clicked()), this, SLOT(loadGSM()));

}

void MTTControlWidget::draw(QPainter* painter, int offset, float scale) {
	if (mtt == NULL) return;

    mtt->draw(painter, offset, scale);
}

void MTTControlWidget::updateRoads(int value) {
	if (mtt == NULL) return;
	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::prevRoads() {
	if (mtt == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::nextRoads() {
	if (mtt == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::loadGSM() {
	if (mtt != NULL) {
		delete mtt;
		mtt = NULL;
	}

	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		mtt = new MTT(filename.toUtf8().data());
		mtt->buildTree();
		ui.horizontalSlider->setMaximum(mtt->sequence.size() - 1);
	}
}
