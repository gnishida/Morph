#ifndef MORPH_H
#define MORPH_H

#include <QtGui/QMainWindow>
#include "RoadGraph.h"
#include "BBox.h"
#include "BFS.h"
#include "ui_Morph.h"
#include "Canvas.h"
#include "BFSControlWidget.h"
#include "BFS2ControlWidget.h"
#include "BFSMultiControlWidget.h"
#include "BFSPropControlWidget.h"
#include <qmap.h>

class Morph : public QMainWindow {
	Q_OBJECT

public:
	Canvas* canvas;

	BFSControlWidget* widgetBFS;
	BFS2ControlWidget* widgetBFS2;
	BFSMultiControlWidget* widgetBFSMulti;
	BFSPropControlWidget* widgetBFSProp;

public:
	Morph(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Morph();

protected:
    void paintEvent(QPaintEvent* event);

private:
	Ui::MorphClass ui;

private slots:
	void startBFS();
	void startBFS2();
	void startBFSMulti();
	void startBFSProp();
	void zoomIn();
	void zoomOut();
};


#endif // MORPH_H
