/********************************************************************************
** Form generated from reading UI file 'version.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VERSION_H
#define UI_VERSION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_version
{
public:

    void setupUi(QWidget *version)
    {
        if (version->objectName().isEmpty())
            version->setObjectName(QString::fromUtf8("version"));
        version->resize(583, 425);

        retranslateUi(version);

        QMetaObject::connectSlotsByName(version);
    } // setupUi

    void retranslateUi(QWidget *version)
    {
        version->setWindowTitle(QApplication::translate("version", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class version: public Ui_version {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VERSION_H
