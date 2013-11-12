#pragma once

#include <qdockwidget.h>
#include "ui_BFSControlWidget.h"
#include "BFSProp.h"

class Morph;

class BFSPropControlWidget : public QDockWidget {
Q_OBJECT

private:
	Morph* parent;
	Ui::BFSControlWidget ui;
	BFSProp* bfs;

public:
	BFSPropControlWidget(Morph* parent);

	void draw(QPainter* painter, int offset, float scale);

public slots:
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

