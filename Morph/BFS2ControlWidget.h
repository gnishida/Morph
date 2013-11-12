#pragma once

#include <qdockwidget.h>
#include "ui_BFSControlWidget.h"
#include "BFS2.h"

class Morph;

class BFS2ControlWidget : public QDockWidget {
Q_OBJECT

private:
	Morph* parent;
	Ui::BFSControlWidget ui;
	BFS2* bfs;

public:
	BFS2ControlWidget(Morph* parent);

	void draw(QPainter* painter, int offset, float scale);

public slots:
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

