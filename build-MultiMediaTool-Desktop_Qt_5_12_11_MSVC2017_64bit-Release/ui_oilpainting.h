/********************************************************************************
** Form generated from reading UI file 'oilpainting.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OILPAINTING_H
#define UI_OILPAINTING_H

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

class Ui_oilPainting
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpinBox *spinBox_detail;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_intensity;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *oilPainting)
    {
        if (oilPainting->objectName().isEmpty())
            oilPainting->setObjectName(QString::fromUtf8("oilPainting"));
        oilPainting->resize(400, 200);
        verticalLayout = new QVBoxLayout(oilPainting);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(oilPainting);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        spinBox_detail = new QSpinBox(oilPainting);
        spinBox_detail->setObjectName(QString::fromUtf8("spinBox_detail"));
        spinBox_detail->setMinimum(1);
        spinBox_detail->setMaximum(20);
        spinBox_detail->setValue(3);

        horizontalLayout_2->addWidget(spinBox_detail);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(oilPainting);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        doubleSpinBox_intensity = new QDoubleSpinBox(oilPainting);
        doubleSpinBox_intensity->setObjectName(QString::fromUtf8("doubleSpinBox_intensity"));
        doubleSpinBox_intensity->setMinimum(0.100000000000000);
        doubleSpinBox_intensity->setMaximum(10.000000000000000);
        doubleSpinBox_intensity->setSingleStep(0.100000000000000);
        doubleSpinBox_intensity->setValue(0.500000000000000);

        horizontalLayout->addWidget(doubleSpinBox_intensity);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(oilPainting);

        QMetaObject::connectSlotsByName(oilPainting);
    } // setupUi

    void retranslateUi(QWidget *oilPainting)
    {
        oilPainting->setWindowTitle(QApplication::translate("oilPainting", "Form", nullptr));
        label_2->setText(QApplication::translate("oilPainting", "\346\262\271\347\224\273\347\273\206\350\212\202 (1-20):", nullptr));
        label->setText(QApplication::translate("oilPainting", "\346\262\271\347\224\273\345\274\272\345\272\246 (0.1-10.0):", nullptr));
    } // retranslateUi

};

namespace Ui {
    class oilPainting: public Ui_oilPainting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OILPAINTING_H
