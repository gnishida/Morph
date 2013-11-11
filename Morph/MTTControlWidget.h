#pragma once

#include <qdockwidget.h>
#include "ui_MTTControlWidget.h"
#include "MTT.h"

class Morph;

class MTTControlWidget : public QDockWidget {
Q_OBJECT

private:
	Morph* parent;
	Ui::MTTControlWidget ui;
	MTT* mtt;

public:
	MTTControlWidget(Morph* parent);

	void draw(QPainter* painter, int offset, float scale);

public slots:
	void updateRoads(int value);
	void prevRoads();
	void nextRoads();
	void loadGSM();
};

