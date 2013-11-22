#pragma once

#include "ControlWidget.h"
#include "ui_BFSControlWidget.h"
#include "BFS2.h"

class Morph;

class BFS2ControlWidget : public ControlWidget {
Q_OBJECT

private:
	Ui::BFSControlWidget ui;
	BFS2* bfs;

public:
	BFS2ControlWidget(Morph* parent);

	void draw(QPainter* painter);

public slots:
	void loadRoad1();
	void loadRoad2();
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

