#include "Canvas.h"
#include <qdockwidget.h>
#include <qpainter.h>
#include <qdebug.h>

Canvas::Canvas(QWidget *parent) : QLabel(parent) {
	controlWidget = NULL;

	scale = 1.0f;
	this->setMinimumSize(10000, 10000);
}

Canvas::~Canvas() {
}

void Canvas::paintEvent(QPaintEvent* event) {
	if (controlWidget == NULL) return;

	QPainter painter(this);
	painter.scale(scale, scale);
	painter.translate(5000.0f, 5000.0f);
	controlWidget->draw(&painter);
}

void Canvas::mousePressEvent(QMouseEvent *event) {
	qDebug() << event->pos().x() << "," << event->pos().y();

	if (event->buttons() == Qt::RightButton) {
		prevMousePos = event->pos();
	}
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
	if (event->buttons() == Qt::RightButton) {
		zoom((event->pos().y() - prevMousePos.y()));
		prevMousePos = event->pos();
	}
}

void Canvas::setControlWidget(ControlWidget* controlWidget) {
	this->controlWidget = controlWidget;
}

void Canvas::zoom(float scale) {
	if (scale > 0) {
		this->scale *= 1.05f;
	} else {
		this->scale *= 0.95f;
	}
	if (this->scale < 0.01f) this->scale = 0.01f;
	update();
}
