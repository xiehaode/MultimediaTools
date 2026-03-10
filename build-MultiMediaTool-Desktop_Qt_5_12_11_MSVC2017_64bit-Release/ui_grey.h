/********************************************************************************
** Form generated from reading UI file 'grey.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GREY_H
#define UI_GREY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_grey
{
public:

    void setupUi(QWidget *grey)
    {
        if (grey->objectName().isEmpty())
            grey->setObjectName(QString::fromUtf8("grey"));
        grey->resize(400, 300);

        retranslateUi(grey);

        QMetaObject::connectSlotsByName(grey);
    } // setupUi

    void retranslateUi(QWidget *grey)
    {
        grey->setWindowTitle(QApplication::translate("grey", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class grey: public Ui_grey {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GREY_H
