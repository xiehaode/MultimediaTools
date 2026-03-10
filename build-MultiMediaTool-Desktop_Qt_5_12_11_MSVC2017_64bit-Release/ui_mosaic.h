/********************************************************************************
** Form generated from reading UI file 'mosaic.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOSAIC_H
#define UI_MOSAIC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mosaic
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_x;
    QSpinBox *spinBox_x;
    QLabel *label_y;
    QSpinBox *spinBox_y;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_width;
    QSpinBox *spinBox_width;
    QLabel *label_height;
    QSpinBox *spinBox_height;
    QHBoxLayout *horizontalLayout;
    QLabel *label_5;
    QSpinBox *spinBox_blockSize;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *mosaic)
    {
        if (mosaic->objectName().isEmpty())
            mosaic->setObjectName(QString::fromUtf8("mosaic"));
        mosaic->resize(400, 250);
        verticalLayout = new QVBoxLayout(mosaic);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(mosaic);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_x = new QLabel(groupBox);
        label_x->setObjectName(QString::fromUtf8("label_x"));

        horizontalLayout_4->addWidget(label_x);

        spinBox_x = new QSpinBox(groupBox);
        spinBox_x->setObjectName(QString::fromUtf8("spinBox_x"));
        spinBox_x->setMinimum(0);
        spinBox_x->setValue(0);

        horizontalLayout_4->addWidget(spinBox_x);

        label_y = new QLabel(groupBox);
        label_y->setObjectName(QString::fromUtf8("label_y"));

        horizontalLayout_4->addWidget(label_y);

        spinBox_y = new QSpinBox(groupBox);
        spinBox_y->setObjectName(QString::fromUtf8("spinBox_y"));
        spinBox_y->setMinimum(0);
        spinBox_y->setValue(0);

        horizontalLayout_4->addWidget(spinBox_y);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_width = new QLabel(groupBox);
        label_width->setObjectName(QString::fromUtf8("label_width"));

        horizontalLayout_5->addWidget(label_width);

        spinBox_width = new QSpinBox(groupBox);
        spinBox_width->setObjectName(QString::fromUtf8("spinBox_width"));
        spinBox_width->setMinimum(1);
        spinBox_width->setMaximum(9999);
        spinBox_width->setValue(100);

        horizontalLayout_5->addWidget(spinBox_width);

        label_height = new QLabel(groupBox);
        label_height->setObjectName(QString::fromUtf8("label_height"));

        horizontalLayout_5->addWidget(label_height);

        spinBox_height = new QSpinBox(groupBox);
        spinBox_height->setObjectName(QString::fromUtf8("spinBox_height"));
        spinBox_height->setMinimum(1);
        spinBox_height->setMaximum(9999);
        spinBox_height->setValue(100);

        horizontalLayout_5->addWidget(spinBox_height);


        verticalLayout_2->addLayout(horizontalLayout_5);


        verticalLayout->addWidget(groupBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_5 = new QLabel(mosaic);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout->addWidget(label_5);

        spinBox_blockSize = new QSpinBox(mosaic);
        spinBox_blockSize->setObjectName(QString::fromUtf8("spinBox_blockSize"));
        spinBox_blockSize->setMinimum(1);
        spinBox_blockSize->setMaximum(100);
        spinBox_blockSize->setValue(10);

        horizontalLayout->addWidget(spinBox_blockSize);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(mosaic);

        QMetaObject::connectSlotsByName(mosaic);
    } // setupUi

    void retranslateUi(QWidget *mosaic)
    {
        mosaic->setWindowTitle(QApplication::translate("mosaic", "Form", nullptr));
        groupBox->setTitle(QApplication::translate("mosaic", "\351\251\254\350\265\233\345\205\213\345\214\272\345\237\237", nullptr));
        label_x->setText(QApplication::translate("mosaic", "X\345\235\220\346\240\207:", nullptr));
        label_y->setText(QApplication::translate("mosaic", "Y\345\235\220\346\240\207:", nullptr));
        label_width->setText(QApplication::translate("mosaic", "\345\256\275\345\272\246:", nullptr));
        label_height->setText(QApplication::translate("mosaic", "\351\253\230\345\272\246:", nullptr));
        label_5->setText(QApplication::translate("mosaic", "\351\251\254\350\265\233\345\205\213\345\235\227\345\244\247\345\260\217:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class mosaic: public Ui_mosaic {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOSAIC_H
