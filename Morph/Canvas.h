#pragma once

#include <qlabel.h>
#include "ControlWidget.h"

class Canvas : public QLabel {
private:
	ControlWidget* controlWidget;

public:
	Canvas(QWidget *parent);
	~Canvas();

protected:
    void paintEvent(QPaintEvent *);

public:
	void setControlWidget(ControlWidget* controlWidget);
};

