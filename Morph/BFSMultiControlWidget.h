#pragma once

#include <qdockwidget.h>
#include "ui_BFSMultiControlWidget.h"
#include "BFSMulti.h"

class Morph;

class BFSMultiControlWidget : public QDockWidget {
Q_OBJECT

private:
	Morph* parent;
	Ui::BFSMultiControlWidget ui;
	BFSMulti* bfs;

public:
	BFSMultiControlWidget(Morph* parent);

	void draw(QPainter* painter, int offset, float scale);

public slots:
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

