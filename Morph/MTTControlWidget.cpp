#include "MTTControlWidget.h"
#include "Morph.h"

MTTControlWidget::MTTControlWidget(Morph* parent, MTT* mtt) : QDockWidget("MTT Control Widget", (QWidget*)parent) {
	ui.setupUi(this);
	this->parent = parent;
	this->mtt = mtt;

	// setup the UI
	ui.horizontalSlider->setMaximum(mtt->sequence1.size() - 2);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateRoads(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevRoads()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextRoads()));
}

void MTTControlWidget::updateRoads(int value) {
	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::prevRoads() {
	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::nextRoads() {
	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}