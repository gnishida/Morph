/********************************************************************************
** Form generated from reading UI file 'BFS2ControlWidget.ui'
**
** Created: Sun Dec 1 20:30:38 2013
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
#include <QtGui/QLineEdit>
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
    QLineEdit *lineEditRoad2;
    QPushButton *pushButtonLoadRoad1;
    QPushButton *pushButtonLoadRoad2;
    QLineEdit *lineEditRoad1;

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
        horizontalSlider->setGeometry(QRect(10, 160, 121, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(dockWidgetContents);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 190, 41, 31));
        pushButtonNext = new QPushButton(dockWidgetContents);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(90, 190, 41, 31));
        lineEditRoad2 = new QLineEdit(dockWidgetContents);
        lineEditRoad2->setObjectName(QString::fromUtf8("lineEditRoad2"));
        lineEditRoad2->setGeometry(QRect(10, 120, 131, 20));
        pushButtonLoadRoad1 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad1->setObjectName(QString::fromUtf8("pushButtonLoadRoad1"));
        pushButtonLoadRoad1->setGeometry(QRect(10, 10, 131, 31));
        pushButtonLoadRoad2 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad2->setObjectName(QString::fromUtf8("pushButtonLoadRoad2"));
        pushButtonLoadRoad2->setGeometry(QRect(10, 80, 131, 31));
        lineEditRoad1 = new QLineEdit(dockWidgetContents);
        lineEditRoad1->setObjectName(QString::fromUtf8("lineEditRoad1"));
        lineEditRoad1->setGeometry(QRect(10, 50, 131, 20));
        BFS2ControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFS2ControlWidget);

        QMetaObject::connectSlotsByName(BFS2ControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFS2ControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFS2ControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFS2ControlWidget", ">", 0, QApplication::UnicodeUTF8));
        pushButtonLoadRoad1->setText(QApplication::translate("BFS2ControlWidget", "Load Road 1", 0, QApplication::UnicodeUTF8));
        pushButtonLoadRoad2->setText(QApplication::translate("BFS2ControlWidget", "Load Road 2", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFS2ControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFS2ControlWidget: public Ui_BFS2ControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFS2CONTROLWIDGET_H
