#include "MTTControlWidget.h"
#include "Morph.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

MTTControlWidget::MTTControlWidget(Morph* parent) : QDockWidget("MTT Control Widget", (QWidget*)parent) {
	ui.setupUi(this);
	this->parent = parent;
	this->mtt = NULL;

	// setup the UI
	ui.lineEditW1->setText("1");
	ui.lineEditW2->setText("1");
	ui.lineEditW3->setText("1");
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.pushButtonLoadGSM, SIGNAL(clicked()), this, SLOT(loadGSM()));
	connect(ui.pushButtonCollapse, SIGNAL(clicked()), this, SLOT(collapse()));

	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(moveSequence(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevSequence()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextSequence()));

}

void MTTControlWidget::loadGSM() {
	if (mtt != NULL) {
		delete mtt;
		mtt = NULL;
	}

	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		mtt = new MTT(filename.toUtf8().data());

		collapse();
	}
}

void MTTControlWidget::collapse() {
	if (mtt == NULL) {
		QMessageBox msgbox(QMessageBox::Warning, "Error", "No road is loaded.");
		msgbox.exec();
		return;
	}

	mtt->buildTree(ui.lineEditW1->text().toFloat(), ui.lineEditW2->text().toFloat(), ui.lineEditW3->text().toFloat());
	ui.horizontalSlider->setMaximum(mtt->sequence.size() - 1);
	ui.horizontalSlider->setValue(0);
	parent->update();
}

void MTTControlWidget::draw(QPainter* painter, int offset, float scale) {
	if (mtt == NULL) return;

    mtt->draw(painter, offset, scale);
}

void MTTControlWidget::moveSequence(int value) {
	if (mtt == NULL) return;
	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::prevSequence() {
	if (mtt == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}

void MTTControlWidget::nextSequence() {
	if (mtt == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	mtt->selectSequence(value);
	parent->update();
}
