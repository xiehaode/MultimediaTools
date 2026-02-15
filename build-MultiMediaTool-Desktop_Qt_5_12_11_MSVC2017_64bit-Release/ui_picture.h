/********************************************************************************
** Form generated from reading UI file 'picture.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PICTURE_H
#define UI_PICTURE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_picture
{
public:
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_Left;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QCheckBox *gray;
    QCheckBox *TextWatermark;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *customOilPaintApprox;
    QCheckBox *OilPainting2;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *Mosaic;
    QCheckBox *FrostedGlass;
    QHBoxLayout *horizontalLayout_4;
    QCheckBox *SkinSmoothing;
    QCheckBox *whitening;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *whitening2;
    QCheckBox *colorinvert;
    QVBoxLayout *verticalLayout_Right;
    QPushButton *ok;
    QPushButton *cancel;
    QSpacerItem *verticalSpacer_5;
    QPushButton *pushButton;
    QVBoxLayout *verticalLayout_Center;
    QToolButton *exportFile;
    QToolButton *addFile;
    QLabel *label;
    QSpinBox *spinBox;
    QLabel *label_2;
    QSpinBox *spinBox_2;
    QLabel *label_3;
    QLineEdit *lineEdit;
    QSpacerItem *verticalSpacer_3;

    void setupUi(QWidget *picture)
    {
        if (picture->objectName().isEmpty())
            picture->setObjectName(QString::fromUtf8("picture"));
        picture->resize(727, 567);
        horizontalLayout_6 = new QHBoxLayout(picture);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        verticalLayout_Left = new QVBoxLayout();
        verticalLayout_Left->setObjectName(QString::fromUtf8("verticalLayout_Left"));
        groupBox = new QGroupBox(picture);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gray = new QCheckBox(groupBox);
        gray->setObjectName(QString::fromUtf8("gray"));

        horizontalLayout->addWidget(gray);

        TextWatermark = new QCheckBox(groupBox);
        TextWatermark->setObjectName(QString::fromUtf8("TextWatermark"));

        horizontalLayout->addWidget(TextWatermark);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        customOilPaintApprox = new QCheckBox(groupBox);
        customOilPaintApprox->setObjectName(QString::fromUtf8("customOilPaintApprox"));

        horizontalLayout_2->addWidget(customOilPaintApprox);

        OilPainting2 = new QCheckBox(groupBox);
        OilPainting2->setObjectName(QString::fromUtf8("OilPainting2"));

        horizontalLayout_2->addWidget(OilPainting2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        Mosaic = new QCheckBox(groupBox);
        Mosaic->setObjectName(QString::fromUtf8("Mosaic"));

        horizontalLayout_3->addWidget(Mosaic);

        FrostedGlass = new QCheckBox(groupBox);
        FrostedGlass->setObjectName(QString::fromUtf8("FrostedGlass"));

        horizontalLayout_3->addWidget(FrostedGlass);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        SkinSmoothing = new QCheckBox(groupBox);
        SkinSmoothing->setObjectName(QString::fromUtf8("SkinSmoothing"));

        horizontalLayout_4->addWidget(SkinSmoothing);

        whitening = new QCheckBox(groupBox);
        whitening->setObjectName(QString::fromUtf8("whitening"));

        horizontalLayout_4->addWidget(whitening);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        whitening2 = new QCheckBox(groupBox);
        whitening2->setObjectName(QString::fromUtf8("whitening2"));

        horizontalLayout_5->addWidget(whitening2);

        colorinvert = new QCheckBox(groupBox);
        colorinvert->setObjectName(QString::fromUtf8("colorinvert"));

        horizontalLayout_5->addWidget(colorinvert);


        verticalLayout->addLayout(horizontalLayout_5);


        verticalLayout_Left->addWidget(groupBox);


        horizontalLayout_6->addLayout(verticalLayout_Left);

        verticalLayout_Right = new QVBoxLayout();
        verticalLayout_Right->setObjectName(QString::fromUtf8("verticalLayout_Right"));
        ok = new QPushButton(picture);
        ok->setObjectName(QString::fromUtf8("ok"));
        ok->setMinimumSize(QSize(100, 40));

        verticalLayout_Right->addWidget(ok);

        cancel = new QPushButton(picture);
        cancel->setObjectName(QString::fromUtf8("cancel"));
        cancel->setMinimumSize(QSize(100, 40));

        verticalLayout_Right->addWidget(cancel);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_Right->addItem(verticalSpacer_5);

        pushButton = new QPushButton(picture);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout_Right->addWidget(pushButton);


        horizontalLayout_6->addLayout(verticalLayout_Right);

        verticalLayout_Center = new QVBoxLayout();
        verticalLayout_Center->setObjectName(QString::fromUtf8("verticalLayout_Center"));
        exportFile = new QToolButton(picture);
        exportFile->setObjectName(QString::fromUtf8("exportFile"));
        exportFile->setMinimumSize(QSize(120, 80));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/rc/export.svg"), QSize(), QIcon::Normal, QIcon::Off);
        exportFile->setIcon(icon);
        exportFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        verticalLayout_Center->addWidget(exportFile);

        addFile = new QToolButton(picture);
        addFile->setObjectName(QString::fromUtf8("addFile"));
        addFile->setMinimumSize(QSize(120, 80));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/rc/addFile.svg"), QSize(), QIcon::Normal, QIcon::Off);
        addFile->setIcon(icon1);
        addFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        verticalLayout_Center->addWidget(addFile);

        label = new QLabel(picture);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_Center->addWidget(label);

        spinBox = new QSpinBox(picture);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));

        verticalLayout_Center->addWidget(spinBox);

        label_2 = new QLabel(picture);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_Center->addWidget(label_2);

        spinBox_2 = new QSpinBox(picture);
        spinBox_2->setObjectName(QString::fromUtf8("spinBox_2"));

        verticalLayout_Center->addWidget(spinBox_2);

        label_3 = new QLabel(picture);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_Center->addWidget(label_3);

        lineEdit = new QLineEdit(picture);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy);

        verticalLayout_Center->addWidget(lineEdit);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_Center->addItem(verticalSpacer_3);


        horizontalLayout_6->addLayout(verticalLayout_Center);


        retranslateUi(picture);

        QMetaObject::connectSlotsByName(picture);
    } // setupUi

    void retranslateUi(QWidget *picture)
    {
        picture->setWindowTitle(QApplication::translate("picture", "Form", nullptr));
        groupBox->setTitle(QApplication::translate("picture", "\347\211\271\346\225\210", nullptr));
        gray->setText(QApplication::translate("picture", "\347\201\260\345\272\246", nullptr));
        TextWatermark->setText(QApplication::translate("picture", "\346\226\207\345\255\227\346\260\264\345\215\260", nullptr));
        customOilPaintApprox->setText(QApplication::translate("picture", "\347\256\200\346\230\223\346\262\271\347\224\273", nullptr));
        OilPainting2->setText(QApplication::translate("picture", "\346\240\207\345\207\206\346\262\271\347\224\273", nullptr));
        Mosaic->setText(QApplication::translate("picture", "\351\251\254\350\265\233\345\205\213", nullptr));
        FrostedGlass->setText(QApplication::translate("picture", "\346\257\233\347\216\273\347\222\203", nullptr));
        SkinSmoothing->setText(QApplication::translate("picture", "\347\243\250\347\232\256", nullptr));
        whitening->setText(QApplication::translate("picture", "\347\276\216\347\231\2751", nullptr));
        whitening2->setText(QApplication::translate("picture", "\347\276\216\347\231\2752", nullptr));
        colorinvert->setText(QApplication::translate("picture", "\350\211\262\345\275\251\345\217\215\350\275\254", nullptr));
        ok->setText(QApplication::translate("picture", "\347\241\256\345\256\232", nullptr));
        cancel->setText(QApplication::translate("picture", "\345\217\226\346\266\210", nullptr));
        pushButton->setText(QApplication::translate("picture", "\346\210\252\345\261\217", nullptr));
        exportFile->setText(QApplication::translate("picture", "\350\276\223\345\207\272\344\275\215\347\275\256", nullptr));
        addFile->setText(QApplication::translate("picture", "\345\276\205\345\244\204\347\220\206\346\226\207\344\273\266", nullptr));
        label->setText(QApplication::translate("picture", "\345\217\202\346\225\260\344\270\200", nullptr));
        label_2->setText(QApplication::translate("picture", "\345\217\202\346\225\260\344\272\214", nullptr));
        label_3->setText(QApplication::translate("picture", "\345\217\202\346\225\260\344\270\211", nullptr));
    } // retranslateUi

};

namespace Ui {
    class picture: public Ui_picture {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PICTURE_H
