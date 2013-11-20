/********************************************************************************
** Form generated from reading UI file 'BFS2ControlWidget.ui'
**
** Created: Wed Nov 20 09:20:12 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BFS2CONTROLWIDGET_H
#define UI_BFS2CONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BFS2ControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;

    void setupUi(QDockWidget *BFS2ControlWidget)
    {
        if (BFS2ControlWidget->objectName().isEmpty())
            BFS2ControlWidget->setObjectName(QString::fromUtf8("BFS2ControlWidget"));
        BFS2ControlWidget->resize(145, 261);
        BFS2ControlWidget->setMinimumSize(QSize(145, 261));
        BFS2ControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        horizontalSlider = new QSlider(dockWidgetContents);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(10, 20, 121, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(dockWidgetContents);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 50, 41, 31));
        pushButtonNext = new QPushButton(dockWidgetContents);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(90, 50, 41, 31));
        BFS2ControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFS2ControlWidget);

        QMetaObject::connectSlotsByName(BFS2ControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFS2ControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFS2ControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFS2ControlWidget", ">", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFS2ControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFS2ControlWidget: public Ui_BFS2ControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFS2CONTROLWIDGET_H
