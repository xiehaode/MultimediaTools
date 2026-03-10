/********************************************************************************
** Form generated from reading UI file 'white1.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WHITE1_H
#define UI_WHITE1_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_white1
{
public:

    void setupUi(QWidget *white1)
    {
        if (white1->objectName().isEmpty())
            white1->setObjectName(QString::fromUtf8("white1"));
        white1->resize(400, 300);

        retranslateUi(white1);

        QMetaObject::connectSlotsByName(white1);
    } // setupUi

    void retranslateUi(QWidget *white1)
    {
        white1->setWindowTitle(QApplication::translate("white1", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class white1: public Ui_white1 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WHITE1_H
