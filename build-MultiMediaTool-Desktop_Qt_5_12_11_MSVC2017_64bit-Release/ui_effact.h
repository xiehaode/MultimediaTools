/********************************************************************************
** Form generated from reading UI file 'effact.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EFFACT_H
#define UI_EFFACT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
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

class Ui_effact
{
public:
    QVBoxLayout *verticalLayout_8;
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout_8;
    QGroupBox *groupBox;
    QWidget *layoutWidget;
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
    QVBoxLayout *verticalLayout_3;
    QToolButton *addFile;
    QToolButton *exportFile;
    QVBoxLayout *verticalLayout_2;
    QSpinBox *spinBoxArgs1;
    QSpinBox *spinBoxArgs2;
    QSpinBox *spinBoxArgs3;
    QSpinBox *spinBoxArgs4;
    QSpinBox *spinBoxArgs5;
    QDoubleSpinBox *doubleSpinBoxArgs1;
    QLineEdit *argLine1;
    QPushButton *ok;
    QPushButton *cancel;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_7;
    QLineEdit *lineEdit_Path;
    QPushButton *btnImport;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_4;
    QLabel *labelSrc;
    QComboBox *comboBoxSrc;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout_5;
    QLabel *labelDst;
    QComboBox *comboBoxDst;
    QPushButton *pushButton;

    void setupUi(QWidget *effact)
    {
        if (effact->objectName().isEmpty())
            effact->setObjectName(QString::fromUtf8("effact"));
        effact->resize(1002, 790);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(effact->sizePolicy().hasHeightForWidth());
        effact->setSizePolicy(sizePolicy);
        verticalLayout_8 = new QVBoxLayout(effact);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        groupBox = new QGroupBox(effact);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        layoutWidget = new QWidget(groupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 40, 251, 261));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setSpacing(14);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gray = new QCheckBox(layoutWidget);
        gray->setObjectName(QString::fromUtf8("gray"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(gray->sizePolicy().hasHeightForWidth());
        gray->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(gray);

        TextWatermark = new QCheckBox(layoutWidget);
        TextWatermark->setObjectName(QString::fromUtf8("TextWatermark"));
        sizePolicy2.setHeightForWidth(TextWatermark->sizePolicy().hasHeightForWidth());
        TextWatermark->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(TextWatermark);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        customOilPaintApprox = new QCheckBox(layoutWidget);
        customOilPaintApprox->setObjectName(QString::fromUtf8("customOilPaintApprox"));

        horizontalLayout_2->addWidget(customOilPaintApprox);

        OilPainting2 = new QCheckBox(layoutWidget);
        OilPainting2->setObjectName(QString::fromUtf8("OilPainting2"));
        sizePolicy2.setHeightForWidth(OilPainting2->sizePolicy().hasHeightForWidth());
        OilPainting2->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(OilPainting2);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        Mosaic = new QCheckBox(layoutWidget);
        Mosaic->setObjectName(QString::fromUtf8("Mosaic"));
        sizePolicy2.setHeightForWidth(Mosaic->sizePolicy().hasHeightForWidth());
        Mosaic->setSizePolicy(sizePolicy2);

        horizontalLayout_3->addWidget(Mosaic);

        FrostedGlass = new QCheckBox(layoutWidget);
        FrostedGlass->setObjectName(QString::fromUtf8("FrostedGlass"));
        sizePolicy2.setHeightForWidth(FrostedGlass->sizePolicy().hasHeightForWidth());
        FrostedGlass->setSizePolicy(sizePolicy2);

        horizontalLayout_3->addWidget(FrostedGlass);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        SkinSmoothing = new QCheckBox(layoutWidget);
        SkinSmoothing->setObjectName(QString::fromUtf8("SkinSmoothing"));
        sizePolicy2.setHeightForWidth(SkinSmoothing->sizePolicy().hasHeightForWidth());
        SkinSmoothing->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(SkinSmoothing);

        whitening = new QCheckBox(layoutWidget);
        whitening->setObjectName(QString::fromUtf8("whitening"));
        sizePolicy2.setHeightForWidth(whitening->sizePolicy().hasHeightForWidth());
        whitening->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(whitening);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        whitening2 = new QCheckBox(layoutWidget);
        whitening2->setObjectName(QString::fromUtf8("whitening2"));
        sizePolicy2.setHeightForWidth(whitening2->sizePolicy().hasHeightForWidth());
        whitening2->setSizePolicy(sizePolicy2);

        horizontalLayout_5->addWidget(whitening2);

        colorinvert = new QCheckBox(layoutWidget);
        colorinvert->setObjectName(QString::fromUtf8("colorinvert"));
        sizePolicy2.setHeightForWidth(colorinvert->sizePolicy().hasHeightForWidth());
        colorinvert->setSizePolicy(sizePolicy2);

        horizontalLayout_5->addWidget(colorinvert);


        verticalLayout->addLayout(horizontalLayout_5);


        horizontalLayout_8->addWidget(groupBox);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        addFile = new QToolButton(effact);
        addFile->setObjectName(QString::fromUtf8("addFile"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(addFile->sizePolicy().hasHeightForWidth());
        addFile->setSizePolicy(sizePolicy3);
        addFile->setMinimumSize(QSize(120, 80));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/rc/addFile.svg"), QSize(), QIcon::Normal, QIcon::Off);
        addFile->setIcon(icon);

        verticalLayout_3->addWidget(addFile);

        exportFile = new QToolButton(effact);
        exportFile->setObjectName(QString::fromUtf8("exportFile"));
        sizePolicy3.setHeightForWidth(exportFile->sizePolicy().hasHeightForWidth());
        exportFile->setSizePolicy(sizePolicy3);
        exportFile->setMinimumSize(QSize(120, 80));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/rc/export.svg"), QSize(), QIcon::Normal, QIcon::Off);
        exportFile->setIcon(icon1);

        verticalLayout_3->addWidget(exportFile);


        horizontalLayout_8->addLayout(verticalLayout_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        spinBoxArgs1 = new QSpinBox(effact);
        spinBoxArgs1->setObjectName(QString::fromUtf8("spinBoxArgs1"));

        verticalLayout_2->addWidget(spinBoxArgs1);

        spinBoxArgs2 = new QSpinBox(effact);
        spinBoxArgs2->setObjectName(QString::fromUtf8("spinBoxArgs2"));

        verticalLayout_2->addWidget(spinBoxArgs2);

        spinBoxArgs3 = new QSpinBox(effact);
        spinBoxArgs3->setObjectName(QString::fromUtf8("spinBoxArgs3"));

        verticalLayout_2->addWidget(spinBoxArgs3);

        spinBoxArgs4 = new QSpinBox(effact);
        spinBoxArgs4->setObjectName(QString::fromUtf8("spinBoxArgs4"));

        verticalLayout_2->addWidget(spinBoxArgs4);

        spinBoxArgs5 = new QSpinBox(effact);
        spinBoxArgs5->setObjectName(QString::fromUtf8("spinBoxArgs5"));

        verticalLayout_2->addWidget(spinBoxArgs5);

        doubleSpinBoxArgs1 = new QDoubleSpinBox(effact);
        doubleSpinBoxArgs1->setObjectName(QString::fromUtf8("doubleSpinBoxArgs1"));

        verticalLayout_2->addWidget(doubleSpinBoxArgs1);

        argLine1 = new QLineEdit(effact);
        argLine1->setObjectName(QString::fromUtf8("argLine1"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(argLine1->sizePolicy().hasHeightForWidth());
        argLine1->setSizePolicy(sizePolicy4);
        argLine1->setMinimumSize(QSize(300, 0));

        verticalLayout_2->addWidget(argLine1);

        ok = new QPushButton(effact);
        ok->setObjectName(QString::fromUtf8("ok"));

        verticalLayout_2->addWidget(ok);

        cancel = new QPushButton(effact);
        cancel->setObjectName(QString::fromUtf8("cancel"));

        verticalLayout_2->addWidget(cancel);


        horizontalLayout_8->addLayout(verticalLayout_2);


        verticalLayout_7->addLayout(horizontalLayout_8);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        lineEdit_Path = new QLineEdit(effact);
        lineEdit_Path->setObjectName(QString::fromUtf8("lineEdit_Path"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Maximum);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(lineEdit_Path->sizePolicy().hasHeightForWidth());
        lineEdit_Path->setSizePolicy(sizePolicy5);
        lineEdit_Path->setReadOnly(true);

        horizontalLayout_7->addWidget(lineEdit_Path);

        btnImport = new QPushButton(effact);
        btnImport->setObjectName(QString::fromUtf8("btnImport"));
        sizePolicy4.setHeightForWidth(btnImport->sizePolicy().hasHeightForWidth());
        btnImport->setSizePolicy(sizePolicy4);

        horizontalLayout_7->addWidget(btnImport);


        verticalLayout_6->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        labelSrc = new QLabel(effact);
        labelSrc->setObjectName(QString::fromUtf8("labelSrc"));
        QSizePolicy sizePolicy6(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(labelSrc->sizePolicy().hasHeightForWidth());
        labelSrc->setSizePolicy(sizePolicy6);

        verticalLayout_4->addWidget(labelSrc);

        comboBoxSrc = new QComboBox(effact);
        comboBoxSrc->setObjectName(QString::fromUtf8("comboBoxSrc"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(comboBoxSrc->sizePolicy().hasHeightForWidth());
        comboBoxSrc->setSizePolicy(sizePolicy7);

        verticalLayout_4->addWidget(comboBoxSrc);


        horizontalLayout_6->addLayout(verticalLayout_4);

        horizontalSpacer = new QSpacerItem(13, 42, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        labelDst = new QLabel(effact);
        labelDst->setObjectName(QString::fromUtf8("labelDst"));
        QSizePolicy sizePolicy8(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(labelDst->sizePolicy().hasHeightForWidth());
        labelDst->setSizePolicy(sizePolicy8);

        verticalLayout_5->addWidget(labelDst);

        comboBoxDst = new QComboBox(effact);
        comboBoxDst->setObjectName(QString::fromUtf8("comboBoxDst"));
        sizePolicy7.setHeightForWidth(comboBoxDst->sizePolicy().hasHeightForWidth());
        comboBoxDst->setSizePolicy(sizePolicy7);

        verticalLayout_5->addWidget(comboBoxDst);


        horizontalLayout_6->addLayout(verticalLayout_5);


        verticalLayout_6->addLayout(horizontalLayout_6);

        pushButton = new QPushButton(effact);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMinimumSize(QSize(0, 40));

        verticalLayout_6->addWidget(pushButton);


        verticalLayout_7->addLayout(verticalLayout_6);


        verticalLayout_8->addLayout(verticalLayout_7);


        retranslateUi(effact);

        QMetaObject::connectSlotsByName(effact);
    } // setupUi

    void retranslateUi(QWidget *effact)
    {
        effact->setWindowTitle(QApplication::translate("effact", "Form", nullptr));
        groupBox->setTitle(QApplication::translate("effact", "\347\211\271\346\225\210", nullptr));
        gray->setText(QApplication::translate("effact", "\347\201\260\345\272\246", nullptr));
        TextWatermark->setText(QApplication::translate("effact", "\346\226\207\345\255\227\346\260\264\345\215\260", nullptr));
        customOilPaintApprox->setText(QApplication::translate("effact", "\347\256\200\346\230\223\346\262\271\347\224\273", nullptr));
        OilPainting2->setText(QApplication::translate("effact", "\346\240\207\345\207\206\346\262\271\347\224\273", nullptr));
        Mosaic->setText(QApplication::translate("effact", "\351\251\254\350\265\233\345\205\213", nullptr));
        FrostedGlass->setText(QApplication::translate("effact", "\346\257\233\347\216\273\347\222\203", nullptr));
        SkinSmoothing->setText(QApplication::translate("effact", "\347\243\250\347\232\256", nullptr));
        whitening->setText(QApplication::translate("effact", "\347\276\216\347\231\2751", nullptr));
        whitening2->setText(QApplication::translate("effact", "\347\276\216\347\231\2752", nullptr));
        colorinvert->setText(QApplication::translate("effact", "\350\211\262\345\275\251\345\217\215\350\275\254", nullptr));
        addFile->setText(QApplication::translate("effact", "\345\276\205\345\244\204\347\220\206\346\226\207\344\273\266", nullptr));
        exportFile->setText(QApplication::translate("effact", "\350\276\223\345\207\272\344\275\215\347\275\256", nullptr));
        ok->setText(QApplication::translate("effact", "\347\241\256\345\256\232", nullptr));
        cancel->setText(QApplication::translate("effact", "\345\217\226\346\266\210", nullptr));
        lineEdit_Path->setPlaceholderText(QApplication::translate("effact", "\350\257\267\351\200\211\346\213\251\346\210\226\346\213\226\345\205\245\350\246\201\350\275\254\346\215\242\347\232\204\346\226\207\344\273\266...", nullptr));
        btnImport->setText(QApplication::translate("effact", "\345\257\274\345\205\245\346\226\207\344\273\266", nullptr));
        labelSrc->setText(QApplication::translate("effact", "\346\272\220\347\261\273\345\236\213", nullptr));
        labelDst->setText(QApplication::translate("effact", "\347\233\256\346\240\207\347\261\273\345\236\213", nullptr));
        pushButton->setText(QApplication::translate("effact", "\345\274\200\345\247\213\350\275\254\346\215\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class effact: public Ui_effact {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EFFACT_H
