/********************************************************************************
** Form generated from reading UI file 'smooth.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SMOOTH_H
#define UI_SMOOTH_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_smooth
{
public:

    void setupUi(QWidget *smooth)
    {
        if (smooth->objectName().isEmpty())
            smooth->setObjectName(QString::fromUtf8("smooth"));
        smooth->resize(400, 300);

        retranslateUi(smooth);

        QMetaObject::connectSlotsByName(smooth);
    } // setupUi

    void retranslateUi(QWidget *smooth)
    {
        smooth->setWindowTitle(QApplication::translate("smooth", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class smooth: public Ui_smooth {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SMOOTH_H
