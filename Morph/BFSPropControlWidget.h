#pragma once

#include "ControlWidget.h"
#include "ui_BFSPropControlWidget.h"
#include "BFSProp.h"

class Morph;

class BFSPropControlWidget : public ControlWidget {
Q_OBJECT

private:
	Ui::BFSPropControlWidget ui;
	BFSProp* bfs;

public:
	BFSPropControlWidget(Morph* parent);

	void draw(QPainter* painter);
	void selectVertex(float x, float y);

public slots:
	void loadRoad1();
	void loadRoad2();
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

