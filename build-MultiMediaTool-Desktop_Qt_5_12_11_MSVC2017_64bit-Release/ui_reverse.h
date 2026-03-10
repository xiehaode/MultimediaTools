/********************************************************************************
** Form generated from reading UI file 'reverse.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REVERSE_H
#define UI_REVERSE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_reverse
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *reverse)
    {
        if (reverse->objectName().isEmpty())
            reverse->setObjectName(QString::fromUtf8("reverse"));
        reverse->resize(400, 350);
        verticalLayout = new QVBoxLayout(reverse);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

        retranslateUi(reverse);

        QMetaObject::connectSlotsByName(reverse);
    } // setupUi

    void retranslateUi(QWidget *reverse)
    {
        reverse->setWindowTitle(QApplication::translate("reverse", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class reverse: public Ui_reverse {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REVERSE_H
