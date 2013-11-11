/********************************************************************************
** Form generated from reading UI file 'BFSControlWidget.ui'
**
** Created: Mon Nov 11 11:16:02 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BFSCONTROLWIDGET_H
#define UI_BFSCONTROLWIDGET_H

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

class Ui_BFSControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSlider;
    QPushButton *pushButtonPrev;
    QPushButton *pushButtonNext;

    void setupUi(QDockWidget *BFSControlWidget)
    {
        if (BFSControlWidget->objectName().isEmpty())
            BFSControlWidget->setObjectName(QString::fromUtf8("BFSControlWidget"));
        BFSControlWidget->resize(145, 261);
        BFSControlWidget->setMinimumSize(QSize(145, 261));
        BFSControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
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
        BFSControlWidget->setWidget(dockWidgetContents);

        retranslateUi(BFSControlWidget);

        QMetaObject::connectSlotsByName(BFSControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *BFSControlWidget)
    {
        pushButtonPrev->setText(QApplication::translate("BFSControlWidget", "<", 0, QApplication::UnicodeUTF8));
        pushButtonNext->setText(QApplication::translate("BFSControlWidget", ">", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(BFSControlWidget);
    } // retranslateUi

};

namespace Ui {
    class BFSControlWidget: public Ui_BFSControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BFSCONTROLWIDGET_H
