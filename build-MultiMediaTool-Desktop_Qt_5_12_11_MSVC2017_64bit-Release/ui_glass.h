/********************************************************************************
** Form generated from reading UI file 'glass.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLASS_H
#define UI_GLASS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_glass
{
public:

    void setupUi(QWidget *glass)
    {
        if (glass->objectName().isEmpty())
            glass->setObjectName(QString::fromUtf8("glass"));
        glass->resize(400, 300);

        retranslateUi(glass);

        QMetaObject::connectSlotsByName(glass);
    } // setupUi

    void retranslateUi(QWidget *glass)
    {
        glass->setWindowTitle(QApplication::translate("glass", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class glass: public Ui_glass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLASS_H
