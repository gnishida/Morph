/********************************************************************************
** Form generated from reading UI file 'BFSMultiControlWidget.ui'
**
** Created: Thu Nov 21 21:31:53 2013
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
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BFSMultiControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;

    void setupUi(QDockWidget *BFSMultiControlWidget)
    {
        if (BFSMultiControlWidget->objectName().isEmpty())
            BFSMultiControlWidget->setObjectName(QString::fromUtf8("BFSMultiControlWidget"));
        BFSMultiControlWidget->resize(145, 261);
        BFSMultiControlWidget->setMinimumSize(QSize(145, 261));
        BFSMultiControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
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
        BFSMultiControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFSMultiControlWidget);

        QMetaObject::connectSlotsByName(BFSMultiControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFSMultiControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFSMultiControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFSMultiControlWidget", ">", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFSMultiControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFSMultiControlWidget: public Ui_BFSMultiControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFSMULTICONTROLWIDGET_H
