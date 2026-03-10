/********************************************************************************
** Form generated from reading UI file 'watertext.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WATERTEXT_H
#define UI_WATERTEXT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_waterText
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *lineEdit_watermarkText;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_x;
    QSpinBox *spinBox_x;
    QLabel *label_y;
    QSpinBox *spinBox_y;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_scale;
    QDoubleSpinBox *doubleSpinBox_scale;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_thickness;
    QSpinBox *spinBox_thickness;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_r;
    QSpinBox *spinBox_r;
    QLabel *label_g;
    QSpinBox *spinBox_g;
    QLabel *label_b;
    QSpinBox *spinBox_b;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *waterText)
    {
        if (waterText->objectName().isEmpty())
            waterText->setObjectName(QString::fromUtf8("waterText"));
        waterText->resize(400, 350);
        verticalLayout = new QVBoxLayout(waterText);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(waterText);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        lineEdit_watermarkText = new QLineEdit(groupBox);
        lineEdit_watermarkText->setObjectName(QString::fromUtf8("lineEdit_watermarkText"));

        verticalLayout_2->addWidget(lineEdit_watermarkText);


        verticalLayout->addWidget(groupBox);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_x = new QLabel(waterText);
        label_x->setObjectName(QString::fromUtf8("label_x"));

        horizontalLayout_2->addWidget(label_x);

        spinBox_x = new QSpinBox(waterText);
        spinBox_x->setObjectName(QString::fromUtf8("spinBox_x"));
        spinBox_x->setMinimum(0);
        spinBox_x->setMaximum(9999);
        spinBox_x->setValue(100);

        horizontalLayout_2->addWidget(spinBox_x);

        label_y = new QLabel(waterText);
        label_y->setObjectName(QString::fromUtf8("label_y"));

        horizontalLayout_2->addWidget(label_y);

        spinBox_y = new QSpinBox(waterText);
        spinBox_y->setObjectName(QString::fromUtf8("spinBox_y"));
        spinBox_y->setMinimum(0);
        spinBox_y->setMaximum(9999);
        spinBox_y->setValue(100);

        horizontalLayout_2->addWidget(spinBox_y);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_scale = new QLabel(waterText);
        label_scale->setObjectName(QString::fromUtf8("label_scale"));

        horizontalLayout_3->addWidget(label_scale);

        doubleSpinBox_scale = new QDoubleSpinBox(waterText);
        doubleSpinBox_scale->setObjectName(QString::fromUtf8("doubleSpinBox_scale"));
        doubleSpinBox_scale->setMinimum(0.100000000000000);
        doubleSpinBox_scale->setMaximum(10.000000000000000);
        doubleSpinBox_scale->setSingleStep(0.100000000000000);
        doubleSpinBox_scale->setValue(1.000000000000000);

        horizontalLayout_3->addWidget(doubleSpinBox_scale);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_thickness = new QLabel(waterText);
        label_thickness->setObjectName(QString::fromUtf8("label_thickness"));

        horizontalLayout_4->addWidget(label_thickness);

        spinBox_thickness = new QSpinBox(waterText);
        spinBox_thickness->setObjectName(QString::fromUtf8("spinBox_thickness"));
        spinBox_thickness->setMinimum(1);
        spinBox_thickness->setMaximum(10);
        spinBox_thickness->setValue(2);

        horizontalLayout_4->addWidget(spinBox_thickness);


        verticalLayout->addLayout(horizontalLayout_4);

        groupBox_2 = new QGroupBox(waterText);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_r = new QLabel(groupBox_2);
        label_r->setObjectName(QString::fromUtf8("label_r"));

        horizontalLayout_5->addWidget(label_r);

        spinBox_r = new QSpinBox(groupBox_2);
        spinBox_r->setObjectName(QString::fromUtf8("spinBox_r"));
        spinBox_r->setMinimum(0);
        spinBox_r->setMaximum(255);
        spinBox_r->setValue(255);

        horizontalLayout_5->addWidget(spinBox_r);

        label_g = new QLabel(groupBox_2);
        label_g->setObjectName(QString::fromUtf8("label_g"));

        horizontalLayout_5->addWidget(label_g);

        spinBox_g = new QSpinBox(groupBox_2);
        spinBox_g->setObjectName(QString::fromUtf8("spinBox_g"));
        spinBox_g->setMinimum(0);
        spinBox_g->setMaximum(255);
        spinBox_g->setValue(255);

        horizontalLayout_5->addWidget(spinBox_g);

        label_b = new QLabel(groupBox_2);
        label_b->setObjectName(QString::fromUtf8("label_b"));

        horizontalLayout_5->addWidget(label_b);

        spinBox_b = new QSpinBox(groupBox_2);
        spinBox_b->setObjectName(QString::fromUtf8("spinBox_b"));
        spinBox_b->setMinimum(0);
        spinBox_b->setMaximum(255);
        spinBox_b->setValue(255);

        horizontalLayout_5->addWidget(spinBox_b);


        verticalLayout_3->addLayout(horizontalLayout_5);


        verticalLayout->addWidget(groupBox_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(waterText);

        QMetaObject::connectSlotsByName(waterText);
    } // setupUi

    void retranslateUi(QWidget *waterText)
    {
        waterText->setWindowTitle(QApplication::translate("waterText", "\346\226\207\345\255\227\346\260\264\345\215\260", nullptr));
        groupBox->setTitle(QApplication::translate("waterText", "\346\260\264\345\215\260\346\226\207\345\255\227", nullptr));
        lineEdit_watermarkText->setPlaceholderText(QApplication::translate("waterText", "\350\257\267\350\276\223\345\205\245\346\260\264\345\215\260\346\226\207\345\255\227", nullptr));
        label_x->setText(QApplication::translate("waterText", "X\345\235\220\346\240\207:", nullptr));
        label_y->setText(QApplication::translate("waterText", "Y\345\235\220\346\240\207:", nullptr));
        label_scale->setText(QApplication::translate("waterText", "\345\255\227\344\275\223\347\274\251\346\224\276:", nullptr));
        label_thickness->setText(QApplication::translate("waterText", "\346\226\207\345\255\227\347\262\227\347\273\206:", nullptr));
        groupBox_2->setTitle(QApplication::translate("waterText", "\346\226\207\345\255\227\351\242\234\350\211\262", nullptr));
        label_r->setText(QApplication::translate("waterText", "R:", nullptr));
        label_g->setText(QApplication::translate("waterText", "G:", nullptr));
        label_b->setText(QApplication::translate("waterText", "B:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class waterText: public Ui_waterText {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WATERTEXT_H
