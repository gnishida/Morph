#include "Canvas.h"
#include <qdockwidget.h>
#include <qpainter.h>

Canvas::Canvas(QWidget *parent) : QLabel(parent) {
	controlWidget = NULL;

	this->setMinimumSize(1000, 1000);
}

Canvas::~Canvas() {
}

void Canvas::paintEvent(QPaintEvent* event) {
	if (controlWidget == NULL) return;

	QPainter painter(this);
	painter.scale(0.1f, 0.1f);
	painter.translate(5000.0f, 5000.0f);
	controlWidget->draw(&painter);
}  

void Canvas::setControlWidget(ControlWidget* controlWidget) {
	this->controlWidget = controlWidget;
}