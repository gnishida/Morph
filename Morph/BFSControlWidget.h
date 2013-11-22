#pragma once

#include "ControlWidget.h"
#include "ui_BFSControlWidget.h"
#include "BFS.h"

class Morph;

class BFSControlWidget : public ControlWidget {
Q_OBJECT

private:
	Ui::BFSControlWidget ui;
	BFS* bfs;

public:
	BFSControlWidget(Morph* parent);

	void draw(QPainter* painter);

public slots:
	void loadRoad1();
	void loadRoad2();
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

