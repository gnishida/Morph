#pragma once

#include "ControlWidget.h"
#include "ui_BFSMultiControlWidget.h"
#include "BFSMulti.h"

class Morph;

class BFSMultiControlWidget : public ControlWidget {
Q_OBJECT

private:
	Ui::BFSMultiControlWidget ui;
	BFSMulti* bfs;

public:
	BFSMultiControlWidget(Morph* parent);

	void draw(QPainter* painter);
	void selectVertex(float x, float y);

public slots:
	void loadRoad1();
	void loadRoad2();
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

