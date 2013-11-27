/********************************************************************************
** Form generated from reading UI file 'BFSPropControlWidget.ui'
**
** Created: Tue Nov 26 22:43:08 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BFSPROPCONTROLWIDGET_H
#define UI_BFSPROPCONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BFSPropControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;
    QLineEdit *lineEditRoad1;
    QLineEdit *lineEditRoad2;
    QPushButton *pushButtonLoadRoad2;
    QPushButton *pushButtonLoadRoad1;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *lineEditNode;
    QLineEdit *lineEditNeighbors;

    void setupUi(QDockWidget *BFSPropControlWidget)
    {
        if (BFSPropControlWidget->objectName().isEmpty())
            BFSPropControlWidget->setObjectName(QString::fromUtf8("BFSPropControlWidget"));
        BFSPropControlWidget->resize(145, 441);
        BFSPropControlWidget->setMinimumSize(QSize(145, 261));
        BFSPropControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
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
        lineEditRoad1 = new QLineEdit(dockWidgetContents);
        lineEditRoad1->setObjectName(QString::fromUtf8("lineEditRoad1"));
        lineEditRoad1->setGeometry(QRect(10, 50, 131, 20));
        lineEditRoad1->setReadOnly(true);
        lineEditRoad2 = new QLineEdit(dockWidgetContents);
        lineEditRoad2->setObjectName(QString::fromUtf8("lineEditRoad2"));
        lineEditRoad2->setGeometry(QRect(10, 120, 131, 20));
        lineEditRoad2->setReadOnly(true);
        pushButtonLoadRoad2 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad2->setObjectName(QString::fromUtf8("pushButtonLoadRoad2"));
        pushButtonLoadRoad2->setGeometry(QRect(10, 80, 131, 31));
        pushButtonLoadRoad1 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad1->setObjectName(QString::fromUtf8("pushButtonLoadRoad1"));
        pushButtonLoadRoad1->setGeometry(QRect(10, 10, 131, 31));
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 290, 46, 16));
        label_2 = new QLabel(dockWidgetContents);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 320, 51, 21));
        lineEditNode = new QLineEdit(dockWidgetContents);
        lineEditNode->setObjectName(QString::fromUtf8("lineEditNode"));
        lineEditNode->setGeometry(QRect(60, 290, 71, 20));
        lineEditNode->setReadOnly(true);
        lineEditNeighbors = new QLineEdit(dockWidgetContents);
        lineEditNeighbors->setObjectName(QString::fromUtf8("lineEditNeighbors"));
        lineEditNeighbors->setGeometry(QRect(10, 340, 121, 20));
        lineEditNeighbors->setReadOnly(true);
        BFSPropControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFSPropControlWidget);

        QMetaObject::connectSlotsByName(BFSPropControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFSPropControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFSPropControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFSPropControlWidget", ">", 0, QApplication::UnicodeUTF8));
        pushButtonLoadRoad2->setText(QApplication::translate("BFSPropControlWidget", "Load Road 2", 0, QApplication::UnicodeUTF8));
        pushButtonLoadRoad1->setText(QApplication::translate("BFSPropControlWidget", "Load Road 1", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("BFSPropControlWidget", "Node:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("BFSPropControlWidget", "Neighbors:", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFSPropControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFSPropControlWidget: public Ui_BFSPropControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFSPROPCONTROLWIDGET_H
