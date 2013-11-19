/********************************************************************************
** Form generated from reading UI file 'BFSPropControlWidget.ui'
**
** Created: Tue Nov 19 12:07:26 2013
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

    void setupUi(QDockWidget *BFSPropControlWidget)
    {
        if (BFSPropControlWidget->objectName().isEmpty())
            BFSPropControlWidget->setObjectName(QString::fromUtf8("BFSPropControlWidget"));
        BFSPropControlWidget->resize(145, 261);
        BFSPropControlWidget->setMinimumSize(QSize(145, 261));
        BFSPropControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
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
        BFSPropControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFSPropControlWidget);

        QMetaObject::connectSlotsByName(BFSPropControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFSPropControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFSPropControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFSPropControlWidget", ">", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFSPropControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFSPropControlWidget: public Ui_BFSPropControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFSPROPCONTROLWIDGET_H
