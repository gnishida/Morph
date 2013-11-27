/********************************************************************************
** Form generated from reading UI file 'BFSMultiControlWidget.ui'
**
** Created: Tue Nov 26 22:43:08 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BFSMULTICONTROLWIDGET_H
#define UI_BFSMULTICONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDockWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BFSMultiControlWidget
{
public:
    QWidget *dockWidgetContents;
    QLineEdit *lineEditRoad1;
    QLineEdit *lineEditRoad2;
    QPushButton *pushButtonLoadRoad2;
    QPushButton *pushButtonLoadRoad1;
    QPushButton *pushButtonCompute;
    QGroupBox *groupBox;
    QLineEdit *lineEditNodeId;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *lineEditNodeNeighbors;
    QGroupBox *groupBox_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *lineEditEdgeLength;
    QLineEdit *lineEditEdgeLanes;
    QLineEdit *lineEditEdgeGroup;
    QGroupBox *groupBox_3;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;
    QCheckBox *checkBoxRoads1;
    QCheckBox *checkBoxRoads2;
    QCheckBox *checkBoxInterpolation;

    void setupUi(QDockWidget *BFSMultiControlWidget)
    {
        if (BFSMultiControlWidget->objectName().isEmpty())
            BFSMultiControlWidget->setObjectName(QString::fromUtf8("BFSMultiControlWidget"));
        BFSMultiControlWidget->resize(148, 635);
        BFSMultiControlWidget->setMinimumSize(QSize(145, 261));
        BFSMultiControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        lineEditRoad1 = new QLineEdit(dockWidgetContents);
        lineEditRoad1->setObjectName(QString::fromUtf8("lineEditRoad1"));
        lineEditRoad1->setGeometry(QRect(10, 50, 131, 20));
        lineEditRoad2 = new QLineEdit(dockWidgetContents);
        lineEditRoad2->setObjectName(QString::fromUtf8("lineEditRoad2"));
        lineEditRoad2->setGeometry(QRect(10, 120, 131, 20));
        pushButtonLoadRoad2 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad2->setObjectName(QString::fromUtf8("pushButtonLoadRoad2"));
        pushButtonLoadRoad2->setGeometry(QRect(10, 80, 131, 31));
        pushButtonLoadRoad1 = new QPushButton(dockWidgetContents);
        pushButtonLoadRoad1->setObjectName(QString::fromUtf8("pushButtonLoadRoad1"));
        pushButtonLoadRoad1->setGeometry(QRect(10, 10, 131, 31));
        pushButtonCompute = new QPushButton(dockWidgetContents);
        pushButtonCompute->setObjectName(QString::fromUtf8("pushButtonCompute"));
        pushButtonCompute->setGeometry(QRect(10, 150, 131, 31));
        groupBox = new QGroupBox(dockWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 380, 131, 101));
        lineEditNodeId = new QLineEdit(groupBox);
        lineEditNodeId->setObjectName(QString::fromUtf8("lineEditNodeId"));
        lineEditNodeId->setGeometry(QRect(50, 20, 71, 20));
        lineEditNodeId->setReadOnly(true);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 31, 16));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 50, 51, 21));
        lineEditNodeNeighbors = new QLineEdit(groupBox);
        lineEditNodeNeighbors->setObjectName(QString::fromUtf8("lineEditNodeNeighbors"));
        lineEditNodeNeighbors->setGeometry(QRect(10, 70, 111, 20));
        lineEditNodeNeighbors->setReadOnly(true);
        groupBox_2 = new QGroupBox(dockWidgetContents);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 490, 131, 111));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 50, 41, 16));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 80, 46, 16));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 20, 46, 16));
        lineEditEdgeLength = new QLineEdit(groupBox_2);
        lineEditEdgeLength->setObjectName(QString::fromUtf8("lineEditEdgeLength"));
        lineEditEdgeLength->setGeometry(QRect(60, 20, 61, 20));
        lineEditEdgeLanes = new QLineEdit(groupBox_2);
        lineEditEdgeLanes->setObjectName(QString::fromUtf8("lineEditEdgeLanes"));
        lineEditEdgeLanes->setGeometry(QRect(60, 50, 61, 20));
        lineEditEdgeGroup = new QLineEdit(groupBox_2);
        lineEditEdgeGroup->setObjectName(QString::fromUtf8("lineEditEdgeGroup"));
        lineEditEdgeGroup->setGeometry(QRect(60, 80, 61, 20));
        groupBox_3 = new QGroupBox(dockWidgetContents);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 200, 131, 171));
        horizontalSlider = new QSlider(groupBox_3);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(10, 90, 111, 20));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(groupBox_3);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 120, 41, 31));
        pushButtonNext = new QPushButton(groupBox_3);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(80, 120, 41, 31));
        checkBoxRoads1 = new QCheckBox(groupBox_3);
        checkBoxRoads1->setObjectName(QString::fromUtf8("checkBoxRoads1"));
        checkBoxRoads1->setGeometry(QRect(10, 20, 70, 17));
        checkBoxRoads2 = new QCheckBox(groupBox_3);
        checkBoxRoads2->setObjectName(QString::fromUtf8("checkBoxRoads2"));
        checkBoxRoads2->setGeometry(QRect(10, 40, 70, 17));
        checkBoxInterpolation = new QCheckBox(groupBox_3);
        checkBoxInterpolation->setObjectName(QString::fromUtf8("checkBoxInterpolation"));
        checkBoxInterpolation->setGeometry(QRect(10, 60, 91, 17));
        BFSMultiControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFSMultiControlWidget);

        QMetaObject::connectSlotsByName(BFSMultiControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFSMultiControlWidget)
    {
        pushButtonLoadRoad2->setText(QApplication::translate("BFSMultiControlWidget", "Load Road 2", 0, QApplication::UnicodeUTF8));
        pushButtonLoadRoad1->setText(QApplication::translate("BFSMultiControlWidget", "Load Road 1", 0, QApplication::UnicodeUTF8));
        pushButtonCompute->setText(QApplication::translate("BFSMultiControlWidget", "Compute", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("BFSMultiControlWidget", "Node Information", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("BFSMultiControlWidget", "Node:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("BFSMultiControlWidget", "Neighbors:", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("BFSMultiControlWidget", "Edge Information", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("BFSMultiControlWidget", "Lanes:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("BFSMultiControlWidget", "Group:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("BFSMultiControlWidget", "Length:", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("BFSMultiControlWidget", "Show", 0, QApplication::UnicodeUTF8));
        pushButtonPrev->setText(QApplication::translate("BFSMultiControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFSMultiControlWidget", ">", 0, QApplication::UnicodeUTF8));
        checkBoxRoads1->setText(QApplication::translate("BFSMultiControlWidget", "Roads1", 0, QApplication::UnicodeUTF8));
        checkBoxRoads2->setText(QApplication::translate("BFSMultiControlWidget", "Roads2", 0, QApplication::UnicodeUTF8));
        checkBoxInterpolation->setText(QApplication::translate("BFSMultiControlWidget", "Interpolation", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFSMultiControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFSMultiControlWidget: public Ui_BFSMultiControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFSMULTICONTROLWIDGET_H
