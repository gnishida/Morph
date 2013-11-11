#pragma once

#include <qdockwidget.h>
#include "ui_BFSControlWidget.h"
#include "BFS.h"

class Morph;

class BFSControlWidget : public QDockWidget {
Q_OBJECT

private:
	Morph* parent;
	Ui::BFSControlWidget ui;
	BFS* bfs;

public:
	BFSControlWidget(Morph* parent);

	void draw(QPainter* painter, int offset, float scale);

public slots:
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

