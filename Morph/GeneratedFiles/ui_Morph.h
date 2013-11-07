/********************************************************************************
** Form generated from reading UI file 'Morph.ui'
**
** Created: Thu Nov 7 01:05:17 2013
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
    QAction *actionStart;
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
        actionStart = new QAction(MorphClass);
        actionStart->setObjectName(QString::fromUtf8("actionStart"));
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
        menuMorphing->addAction(actionStart);

        retranslateUi(MorphClass);

        QMetaObject::connectSlotsByName(MorphClass);
    } // setupUi

    void retranslateUi(QMainWindow *MorphClass)
    {
        MorphClass->setWindowTitle(QApplication::translate("MorphClass", "Morph", 0, QApplication::UnicodeUTF8));
        actionStart->setText(QApplication::translate("MorphClass", "Start", 0, QApplication::UnicodeUTF8));
        menuMorphing->setTitle(QApplication::translate("MorphClass", "Morphing", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MorphClass: public Ui_MorphClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MORPH_H
