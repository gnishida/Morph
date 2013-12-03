#pragma once

#include <qlabel.h>
#include <QMouseEvent>
#include "ControlWidget.h"

class Canvas : public QLabel {
private:
	int width;
	int height;
	ControlWidget* controlWidget;
	float scale;
	QPoint prevMousePos;

public:
	Canvas(QWidget *parent);
	~Canvas();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

public:
	void setControlWidget(ControlWidget* controlWidget);
	void zoom(float scale, QPoint center);
};

