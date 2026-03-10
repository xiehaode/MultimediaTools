/********************************************************************************
** Form generated from reading UI file 'easyoilpainting.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EASYOILPAINTING_H
#define UI_EASYOILPAINTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_easyOilPainting
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_radius;
    QSpinBox *spinBox_radius;
    QHBoxLayout *horizontalLayout;
    QLabel *label_level;
    QDoubleSpinBox *doubleSpinBox_level;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *easyOilPainting)
    {
        if (easyOilPainting->objectName().isEmpty())
            easyOilPainting->setObjectName(QString::fromUtf8("easyOilPainting"));
        easyOilPainting->resize(400, 200);
        verticalLayout = new QVBoxLayout(easyOilPainting);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_radius = new QLabel(easyOilPainting);
        label_radius->setObjectName(QString::fromUtf8("label_radius"));

        horizontalLayout_2->addWidget(label_radius);

        spinBox_radius = new QSpinBox(easyOilPainting);
        spinBox_radius->setObjectName(QString::fromUtf8("spinBox_radius"));
        spinBox_radius->setMinimum(1);
        spinBox_radius->setMaximum(20);
        spinBox_radius->setValue(3);

        horizontalLayout_2->addWidget(spinBox_radius);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_level = new QLabel(easyOilPainting);
        label_level->setObjectName(QString::fromUtf8("label_level"));

        horizontalLayout->addWidget(label_level);

        doubleSpinBox_level = new QDoubleSpinBox(easyOilPainting);
        doubleSpinBox_level->setObjectName(QString::fromUtf8("doubleSpinBox_level"));
        doubleSpinBox_level->setMinimum(0.100000000000000);
        doubleSpinBox_level->setMaximum(10.000000000000000);
        doubleSpinBox_level->setSingleStep(0.100000000000000);
        doubleSpinBox_level->setValue(0.500000000000000);

        horizontalLayout->addWidget(doubleSpinBox_level);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(easyOilPainting);

        QMetaObject::connectSlotsByName(easyOilPainting);
    } // setupUi

    void retranslateUi(QWidget *easyOilPainting)
    {
        easyOilPainting->setWindowTitle(QApplication::translate("easyOilPainting", "\347\256\200\346\230\223\346\262\271\347\224\273", nullptr));
        label_radius->setText(QApplication::translate("easyOilPainting", "\347\254\224\345\210\267\345\215\212\345\276\204 (1-20):", nullptr));
        label_level->setText(QApplication::translate("easyOilPainting", "\346\262\271\347\224\273\347\250\213\345\272\246 (0.1-10.0):", nullptr));
    } // retranslateUi

};

namespace Ui {
    class easyOilPainting: public Ui_easyOilPainting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EASYOILPAINTING_H
