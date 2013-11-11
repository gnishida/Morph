/********************************************************************************
** Form generated from reading UI file 'MTTControlWidget.ui'
**
** Created: Sun Nov 10 19:08:50 2013
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
    QPushButton *pushButtonLoadGSM;

    void setupUi(QDockWidget *MTTControlWidget)
    {
        if (MTTControlWidget->objectName().isEmpty())
            MTTControlWidget->setObjectName(QString::fromUtf8("MTTControlWidget"));
        MTTControlWidget->resize(145, 199);
        MTTControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(133, 133, 133);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        dockWidgetContents->setMinimumSize(QSize(145, 177));
        horizontalSlider = new QSlider(dockWidgetContents);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(10, 20, 111, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(dockWidgetContents);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 60, 41, 31));
        pushButtonNext = new QPushButton(dockWidgetContents);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(80, 60, 41, 31));
        pushButtonLoadGSM = new QPushButton(dockWidgetContents);
        pushButtonLoadGSM->setObjectName(QString::fromUtf8("pushButtonLoadGSM"));
        pushButtonLoadGSM->setGeometry(QRect(10, 110, 111, 31));
        MTTControlWidget->setWidget(dockWidgetContents);

        retranslateUi(MTTControlWidget);

        QMetaObject::connectSlotsByName(MTTControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *MTTControlWidget)
    {
        MTTControlWidget->setWindowTitle(QApplication::translate("MTTControlWidget", "MTT Control Widget", 0, QApplication::UnicodeUTF8));
        pushButtonPrev->setText(QApplication::translate("MTTControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("MTTControlWidget", ">", 0, QApplication::UnicodeUTF8));
        pushButtonLoadGSM->setText(QApplication::translate("MTTControlWidget", "Load GSM", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MTTControlWidget: public Ui_MTTControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MTTCONTROLWIDGET_H
