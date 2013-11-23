/********************************************************************************
** Form generated from reading UI file 'Morph.ui'
**
** Created: Sat Nov 23 15:28:59 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MORPH_H
#define UI_MORPH_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MorphClass
{
public:
    QAction *actionNearestNeighbor;
    QAction *actionNearestNeighborConnectivity;
    QAction *actionBFS;
    QAction *actionMTT;
    QAction *actionBFS2;
    QAction *actionBFSProp;
    QAction *actionBFSMulti;
    QAction *actionZoomIn;
    QAction *actionZoomOut;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuMorphing;
    QMenu *menuView;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MorphClass)
    {
        if (MorphClass->objectName().isEmpty())
            MorphClass->setObjectName(QString::fromUtf8("MorphClass"));
        MorphClass->resize(774, 604);
        actionNearestNeighbor = new QAction(MorphClass);
        actionNearestNeighbor->setObjectName(QString::fromUtf8("actionNearestNeighbor"));
        actionNearestNeighborConnectivity = new QAction(MorphClass);
        actionNearestNeighborConnectivity->setObjectName(QString::fromUtf8("actionNearestNeighborConnectivity"));
        actionBFS = new QAction(MorphClass);
        actionBFS->setObjectName(QString::fromUtf8("actionBFS"));
        actionMTT = new QAction(MorphClass);
        actionMTT->setObjectName(QString::fromUtf8("actionMTT"));
        actionBFS2 = new QAction(MorphClass);
        actionBFS2->setObjectName(QString::fromUtf8("actionBFS2"));
        actionBFSProp = new QAction(MorphClass);
        actionBFSProp->setObjectName(QString::fromUtf8("actionBFSProp"));
        actionBFSMulti = new QAction(MorphClass);
        actionBFSMulti->setObjectName(QString::fromUtf8("actionBFSMulti"));
        actionZoomIn = new QAction(MorphClass);
        actionZoomIn->setObjectName(QString::fromUtf8("actionZoomIn"));
        actionZoomOut = new QAction(MorphClass);
        actionZoomOut->setObjectName(QString::fromUtf8("actionZoomOut"));
        centralWidget = new QWidget(MorphClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        MorphClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MorphClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 774, 21));
        menuMorphing = new QMenu(menuBar);
        menuMorphing->setObjectName(QString::fromUtf8("menuMorphing"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        MorphClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MorphClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MorphClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MorphClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MorphClass->setStatusBar(statusBar);

        menuBar->addAction(menuMorphing->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuMorphing->addAction(actionBFS);
        menuMorphing->addAction(actionBFS2);
        menuMorphing->addAction(actionBFSMulti);
        menuMorphing->addAction(actionBFSProp);
        menuView->addAction(actionZoomIn);
        menuView->addAction(actionZoomOut);

        retranslateUi(MorphClass);

        QMetaObject::connectSlotsByName(MorphClass);
    } // setupUi

    void retranslateUi(QMainWindow *MorphClass)
    {
        MorphClass->setWindowTitle(QApplication::translate("MorphClass", "Morph", 0, QApplication::UnicodeUTF8));
        actionNearestNeighbor->setText(QApplication::translate("MorphClass", "Nearest Neighbor", 0, QApplication::UnicodeUTF8));
        actionNearestNeighborConnectivity->setText(QApplication::translate("MorphClass", "Nearest Neighbor with Connectivity", 0, QApplication::UnicodeUTF8));
        actionBFS->setText(QApplication::translate("MorphClass", "BFS 1-1", 0, QApplication::UnicodeUTF8));
        actionMTT->setText(QApplication::translate("MorphClass", "MTT", 0, QApplication::UnicodeUTF8));
        actionBFS2->setText(QApplication::translate("MorphClass", "BFS N-N", 0, QApplication::UnicodeUTF8));
        actionBFSProp->setText(QApplication::translate("MorphClass", "BFS Proportional", 0, QApplication::UnicodeUTF8));
        actionBFSMulti->setText(QApplication::translate("MorphClass", "BFS Multi Roots", 0, QApplication::UnicodeUTF8));
        actionZoomIn->setText(QApplication::translate("MorphClass", "Zoom in", 0, QApplication::UnicodeUTF8));
        actionZoomOut->setText(QApplication::translate("MorphClass", "Zoom out", 0, QApplication::UnicodeUTF8));
        menuMorphing->setTitle(QApplication::translate("MorphClass", "Morphing", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("MorphClass", "View", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MorphClass: public Ui_MorphClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MORPH_H
