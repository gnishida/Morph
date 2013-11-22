#include "ControlWidget.h"


ControlWidget::ControlWidget(const char* title, Morph* parent) : QDockWidget(title, (QWidget*)parent) {
	this->parent = parent;
}

ControlWidget::~ControlWidget() {
}
