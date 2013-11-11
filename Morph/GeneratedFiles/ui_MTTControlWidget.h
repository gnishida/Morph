/********************************************************************************
** Form generated from reading UI file 'MTTControlWidget.ui'
**
** Created: Mon Nov 11 09:16:05 2013
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
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
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
    QLineEdit *lineEditW1;
    QLabel *label;
    QLineEdit *lineEditW2;
    QLineEdit *lineEditW3;
    QLabel *label_2;
    QLabel *label_3;
    QPushButton *pushButtonCollapse;

    void setupUi(QDockWidget *MTTControlWidget)
    {
        if (MTTControlWidget->objectName().isEmpty())
            MTTControlWidget->setObjectName(QString::fromUtf8("MTTControlWidget"));
        MTTControlWidget->resize(145, 261);
        MTTControlWidget->setMinimumSize(QSize(145, 222));
        MTTControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        dockWidgetContents->setMinimumSize(QSize(145, 177));
        horizontalSlider = new QSlider(dockWidgetContents);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(10, 160, 111, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonPrev = new QPushButton(dockWidgetContents);
        pushButtonPrev->setObjectName(QString::fromUtf8("pushButtonPrev"));
        pushButtonPrev->setGeometry(QRect(10, 190, 41, 31));
        pushButtonNext = new QPushButton(dockWidgetContents);
        pushButtonNext->setObjectName(QString::fromUtf8("pushButtonNext"));
        pushButtonNext->setGeometry(QRect(80, 190, 41, 31));
        pushButtonLoadGSM = new QPushButton(dockWidgetContents);
        pushButtonLoadGSM->setObjectName(QString::fromUtf8("pushButtonLoadGSM"));
        pushButtonLoadGSM->setGeometry(QRect(10, 10, 111, 31));
        lineEditW1 = new QLineEdit(dockWidgetContents);
        lineEditW1->setObjectName(QString::fromUtf8("lineEditW1"));
        lineEditW1->setGeometry(QRect(60, 50, 61, 20));
        lineEditW1->setStyleSheet(QString::fromUtf8("background-color: rgb(216, 216, 216);"));
        lineEditW1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 50, 21, 16));
        lineEditW2 = new QLineEdit(dockWidgetContents);
        lineEditW2->setObjectName(QString::fromUtf8("lineEditW2"));
        lineEditW2->setGeometry(QRect(60, 70, 61, 20));
        lineEditW2->setStyleSheet(QString::fromUtf8("background-color: rgb(216, 216, 216);"));
        lineEditW2->setFrame(true);
        lineEditW2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lineEditW3 = new QLineEdit(dockWidgetContents);
        lineEditW3->setObjectName(QString::fromUtf8("lineEditW3"));
        lineEditW3->setGeometry(QRect(60, 90, 61, 20));
        lineEditW3->setStyleSheet(QString::fromUtf8("background-color: rgb(216, 216, 216);"));
        lineEditW3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_2 = new QLabel(dockWidgetContents);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 70, 21, 16));
        label_3 = new QLabel(dockWidgetContents);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 90, 21, 16));
        pushButtonCollapse = new QPushButton(dockWidgetContents);
        pushButtonCollapse->setObjectName(QString::fromUtf8("pushButtonCollapse"));
        pushButtonCollapse->setGeometry(QRect(10, 120, 111, 31));
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
        label->setText(QApplication::translate("MTTControlWidget", "w1:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MTTControlWidget", "w2:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MTTControlWidget", "w3:", 0, QApplication::UnicodeUTF8));
        pushButtonCollapse->setText(QApplication::translate("MTTControlWidget", "Collapse", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MTTControlWidget: public Ui_MTTControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MTTCONTROLWIDGET_H
