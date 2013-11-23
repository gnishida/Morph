#include "Canvas.h"
#include <qdockwidget.h>
#include <qpainter.h>
#include <qdebug.h>

Canvas::Canvas(QWidget *parent) : QLabel(parent) {
	width = 10000;
	height = 10000;
	controlWidget = NULL;

	scale = 0.1f;
	this->setMinimumSize(width * this->scale, height * this->scale);
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

	if (event->buttons() == Qt::LeftButton) {
		controlWidget->selectVertex(event->pos().x() / scale - 5000.0f, height - event->pos().y() / scale - 5000.0f);
	} else if (event->buttons() == Qt::RightButton) {
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

	this->setMinimumSize(width * this->scale, height * this->scale);

	update();
	//repaint();
}
