#include "ControlWidget.h"
#include "Morph.h"

ControlWidget::ControlWidget(const char* title, Morph* parent) : QDockWidget(title, (QWidget*)parent) {
	this->parent = parent;
}

ControlWidget::~ControlWidget() {
}

void ControlWidget::update() {
	parent->update();
}
