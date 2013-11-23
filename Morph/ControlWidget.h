#pragma once

#include <qdockwidget.h>

class Morph;

class ControlWidget : public QDockWidget {
protected:
	Morph* parent;

public:
	ControlWidget(const char* title, Morph* parent);
	~ControlWidget();

	virtual void draw(QPainter* painter) = 0;
	void update();
};
