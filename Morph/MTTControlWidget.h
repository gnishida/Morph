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
	MTTControlWidget(Morph* parent, MTT* mtt);

public slots:
	void updateRoads(int value);
	void prevRoads();
	void nextRoads();
};

