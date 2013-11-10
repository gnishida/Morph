/********************************************************************************
** Form generated from reading UI file 'MTTControlWidget.ui'
**
** Created: Sun Nov 10 18:12:21 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MTTCONTROLWIDGET_H
#define UI_MTTCONTROLWIDGET_H

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

class Ui_MTTControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;

    void setupUi(QDockWidget *MTTControlWidget)
    {
        if (MTTControlWidget->objectName().isEmpty())
            MTTControlWidget->setObjectName(QString::fromUtf8("MTTControlWidget"));
        MTTControlWidget->resize(228, 144);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        horizontalSlider = new QSlider(dockWidgetContents);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(10, 20, 201, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(dockWidgetContents);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 60, 71, 31));
        pushButtonNext = new QPushButton(dockWidgetContents);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(140, 60, 71, 31));
        MTTControlWidget->setWidget(dockWidgetContents);

        retranslateUi(MTTControlWidget);

        QMetaObject::connectSlotsByName(MTTControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *MTTControlWidget)
    {
        MTTControlWidget->setWindowTitle(QApplication::translate("MTTControlWidget", "MTT Control Widget", 0, QApplication::UnicodeUTF8));
        pushButtonPrev->setText(QApplication::translate("MTTControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("MTTControlWidget", ">", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MTTControlWidget: public Ui_MTTControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MTTCONTROLWIDGET_H
