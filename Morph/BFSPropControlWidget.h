#pragma once

#include "ControlWidget.h"
#include "ui_BFSControlWidget.h"
#include "BFSProp.h"

class Morph;

class BFSPropControlWidget : public ControlWidget {
Q_OBJECT

private:
	Ui::BFSControlWidget ui;
	BFSProp* bfs;

public:
	BFSPropControlWidget(Morph* parent);

	void draw(QPainter* painter);

public slots:
	void loadRoad1();
	void loadRoad2();
	void moveSequence(int value);
	void prevSequence();
	void nextSequence();
};

