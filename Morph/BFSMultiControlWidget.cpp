#include "BFSMultiControlWidget.h"
#include "Morph.h"
#include "GraphUtil.h"
#include <qfiledialog.h>

BFSMultiControlWidget::BFSMultiControlWidget(Morph* parent) : ControlWidget("BFS Multi Control Widget", parent) {
	ui.setupUi(this);
	this->bfs = NULL;

	// setup the UI
	ui.checkBoxRoads1->setChecked(true);
	ui.checkBoxRoads2->setChecked(true);
	ui.checkBoxInterpolation->setChecked(true);
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setMinimum(0);

	// setup the signal handler
	connect(ui.pushButtonLoadRoad1, SIGNAL(clicked()), this, SLOT(loadRoad1()));
	connect(ui.pushButtonLoadRoad2, SIGNAL(clicked()), this, SLOT(loadRoad2()));
	connect(ui.pushButtonCompute, SIGNAL(clicked()), this, SLOT(compute()));
	connect(ui.checkBoxRoads1, SIGNAL(clicked(bool)), this, SLOT(showRoads1(bool)));
	connect(ui.checkBoxRoads2, SIGNAL(clicked(bool)), this, SLOT(showRoads2(bool)));
	connect(ui.checkBoxInterpolation, SIGNAL(clicked(bool)), this, SLOT(showInterpolation(bool)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(moveSequence(int)));
	connect(ui.pushButtonPrev, SIGNAL(clicked()), this, SLOT(prevSequence()));
	connect(ui.pushButtonNext, SIGNAL(clicked()), this, SLOT(nextSequence()));

	// initialize BFS
	bfs = new BFSMulti();
}

void BFSMultiControlWidget::draw(QPainter* painter) {
	if (bfs == NULL) return;

    bfs->draw(painter);
}

/**
 * 頂点クリック時の処理
 */
void BFSMultiControlWidget::onClick(float x, float y) {
	if (!selectVertex(x, y)) {
		selectEdge(x, y);
	}
}

/**
 * 指定された座標に近い頂点に探し、Widgetに表示する。
 * 見つからない場合は、falseを返却する。
 */
bool BFSMultiControlWidget::selectVertex(float x, float y) {
	RoadGraph* roads = NULL;
	RoadVertexDesc v;

	// クリックされた道路網を調べる
	if (ui.checkBoxRoads1->isChecked() && bfs->roads1 != NULL && GraphUtil::getVertex(bfs->roads1, QVector2D(x, y), 50.0f, v)) {
		roads = bfs->roads1;
	} else if (ui.checkBoxRoads2->isChecked() && bfs->roads2 != NULL && GraphUtil::getVertex(bfs->roads2, QVector2D(x, y), 50.0f, v)) {
		roads = bfs->roads2;
	} else if (ui.checkBoxInterpolation->isChecked() && bfs->getSelectedRoads() != NULL && GraphUtil::getVertex(bfs->getSelectedRoads(), QVector2D(x, y), 50.0f, v)) {
		roads = bfs->getSelectedRoads();
	}

	if (roads == NULL) {
		// 指定した座標の近くに頂点が見つからない場合、Widgetの頂点情報を消す
		ui.lineEditNodeId->setText("");
		ui.lineEditNodeNeighbors->setText("");

		return false;
	}

	QString str;
	ui.lineEditNodeId->setText(str.setNum(v));

	std::vector<RoadVertexDesc> neighbors = GraphUtil::getNeighbors(roads, v);
	str = "";
	for (int i = 0; i < neighbors.size(); i++) {
		QString str2;
		if (str.length() > 0) {
			str = str + ",";
		}
		str = str + str2.setNum(neighbors[i]);
	}
	ui.lineEditNodeNeighbors->setText(str);

	return true;
}

/**
 * 指定された座標に近いエッジを探し、Widgetに表示する。
 * エッジが見つからない場合は、falseを返却する。
 */
bool BFSMultiControlWidget::selectEdge(float x, float y) {
	RoadGraph* roads = NULL;
	RoadEdgeDesc e;

	// クリックされた道路網を調べる
	float dist;
	QVector2D closestPt;
	if (ui.checkBoxRoads1->isChecked() && bfs->roads1 != NULL && GraphUtil::getEdge(bfs->roads1, QVector2D(x, y), 50.0f, e)) {
		roads = bfs->roads1;
	} else if (ui.checkBoxRoads2->isChecked() && bfs->roads2 != NULL && GraphUtil::getEdge(bfs->roads2, QVector2D(x, y), 50.0f, e)) {
		roads = bfs->roads2;
	} else if (ui.checkBoxInterpolation->isChecked() && bfs->getSelectedRoads() != NULL && GraphUtil::getEdge(bfs->getSelectedRoads(), QVector2D(x, y), 50.0f, e)) {
		roads = bfs->getSelectedRoads();
	}

	if (roads == NULL) {
		// 指定した座標の近くにエッジが見つからない場合、Widgetのエッジ情報を消す
		ui.lineEditEdgeLength->setText("");
		ui.lineEditEdgeLanes->setText("");
		ui.lineEditEdgeGroup->setText("");

		return false;
	}

	QString str;
	ui.lineEditEdgeLength->setText(str.setNum(roads->graph[e]->getLength()));
	ui.lineEditEdgeLanes->setText(str.setNum(roads->graph[e]->lanes));
	ui.lineEditEdgeGroup->setText(str.setNum(roads->graph[e]->group));

	return true;
}

void BFSMultiControlWidget::loadRoad1() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		bfs->setRoad1(filename.toUtf8().data());

		ui.lineEditRoad1->setText(filename.split("/").last().split(".").at(0));

		if (bfs->sequence.size() > 0) {
			ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
			ui.horizontalSlider->setValue(0);
			bfs->selectSequence(0);
			update();
		}
	}
}

void BFSMultiControlWidget::loadRoad2() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Load road network ..."), QString(), tr("GSM Files (*.gsm)"));
	if (filename != QString::null && !filename.isEmpty()) {
		bfs->setRoad2(filename.toUtf8().data());

		ui.lineEditRoad2->setText(filename.split("/").last().split(".").at(0));

		if (bfs->sequence.size() > 0) {
			ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
			ui.horizontalSlider->setValue(0);
			bfs->selectSequence(0);
			update();
		}
	}
}

void BFSMultiControlWidget::compute() {
	bfs->init();

	if (bfs->sequence.size() > 0) {
		ui.horizontalSlider->setMaximum(bfs->sequence.size() - 1);
		ui.horizontalSlider->setValue(0);
		bfs->selectSequence(0);
		update();
	}
}

void BFSMultiControlWidget::showRoads1(bool flag) {
	if (bfs != NULL) {
		bfs->showRoads1 = flag;
		update();
	}
}

void BFSMultiControlWidget::showRoads2(bool flag) {
	if (bfs != NULL) {
		bfs->showRoads2 = flag;
		update();
	}
}

void BFSMultiControlWidget::showInterpolation(bool flag) {
	if (bfs != NULL) {
		bfs->showInterpolation = flag;
		update();
	}
}

void BFSMultiControlWidget::moveSequence(int value) {
	if (bfs == NULL) return;
	bfs->selectSequence(value);
	update();
}

void BFSMultiControlWidget::prevSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value > 0) value--;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	update();
}

void BFSMultiControlWidget::nextSequence() {
	if (bfs == NULL) return;

	int value = ui.horizontalSlider->value();
	if (value < ui.horizontalSlider->maximum()) value++;
	ui.horizontalSlider->setValue(value);

	bfs->selectSequence(value);
	update();
}