/********************************************************************************
** Form generated from reading UI file 'Morph.ui'
**
** Created: Thu Nov 7 14:46:39 2013
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
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuMorphing;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MorphClass)
    {
        if (MorphClass->objectName().isEmpty())
            MorphClass->setObjectName(QString::fromUtf8("MorphClass"));
        MorphClass->resize(600, 400);
        actionNearestNeighbor = new QAction(MorphClass);
        actionNearestNeighbor->setObjectName(QString::fromUtf8("actionNearestNeighbor"));
        actionNearestNeighborConnectivity = new QAction(MorphClass);
        actionNearestNeighborConnectivity->setObjectName(QString::fromUtf8("actionNearestNeighborConnectivity"));
        actionBFS = new QAction(MorphClass);
        actionBFS->setObjectName(QString::fromUtf8("actionBFS"));
        actionMTT = new QAction(MorphClass);
        actionMTT->setObjectName(QString::fromUtf8("actionMTT"));
        centralWidget = new QWidget(MorphClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        MorphClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MorphClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        menuMorphing = new QMenu(menuBar);
        menuMorphing->setObjectName(QString::fromUtf8("menuMorphing"));
        MorphClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MorphClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MorphClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MorphClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MorphClass->setStatusBar(statusBar);

        menuBar->addAction(menuMorphing->menuAction());
        menuMorphing->addAction(actionNearestNeighbor);
        menuMorphing->addAction(actionNearestNeighborConnectivity);
        menuMorphing->addAction(actionBFS);
        menuMorphing->addAction(actionMTT);

        retranslateUi(MorphClass);

        QMetaObject::connectSlotsByName(MorphClass);
    } // setupUi

    void retranslateUi(QMainWindow *MorphClass)
    {
        MorphClass->setWindowTitle(QApplication::translate("MorphClass", "Morph", 0, QApplication::UnicodeUTF8));
        actionNearestNeighbor->setText(QApplication::translate("MorphClass", "Nearest Neighbor", 0, QApplication::UnicodeUTF8));
        actionNearestNeighborConnectivity->setText(QApplication::translate("MorphClass", "Nearest Neighbor with Connectivity", 0, QApplication::UnicodeUTF8));
        actionBFS->setText(QApplication::translate("MorphClass", "BFS", 0, QApplication::UnicodeUTF8));
        actionMTT->setText(QApplication::translate("MorphClass", "MTT", 0, QApplication::UnicodeUTF8));
        menuMorphing->setTitle(QApplication::translate("MorphClass", "Morphing", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MorphClass: public Ui_MorphClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MORPH_H
