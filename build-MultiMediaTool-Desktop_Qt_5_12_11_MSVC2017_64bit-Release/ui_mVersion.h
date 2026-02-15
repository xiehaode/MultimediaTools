/********************************************************************************
** Form generated from reading UI file 'mVersion.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MVERSION_H
#define UI_MVERSION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_version
{
public:
    QLabel *appNameLabel;
    QLabel *versionLabel;
    QLabel *copyrightLabel;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *labelBranch;
    QLabel *branchLabel;
    QLabel *labelBuildTime;
    QLabel *buildTimeLabel;
    QLabel *commitLabel;

    void setupUi(QWidget *version)
    {
        if (version->objectName().isEmpty())
            version->setObjectName(QString::fromUtf8("version"));
        version->resize(677, 433);
        appNameLabel = new QLabel(version);
        appNameLabel->setObjectName(QString::fromUtf8("appNameLabel"));
        appNameLabel->setGeometry(QRect(30, 30, 294, 40));
        QFont font;
        font.setPointSize(24);
        font.setBold(true);
        font.setWeight(75);
        appNameLabel->setFont(font);
        appNameLabel->setAlignment(Qt::AlignCenter);
        versionLabel = new QLabel(version);
        versionLabel->setObjectName(QString::fromUtf8("versionLabel"));
        versionLabel->setGeometry(QRect(30, 77, 154, 23));
        QFont font1;
        font1.setPointSize(14);
        versionLabel->setFont(font1);
        versionLabel->setAlignment(Qt::AlignCenter);
        copyrightLabel = new QLabel(version);
        copyrightLabel->setObjectName(QString::fromUtf8("copyrightLabel"));
        copyrightLabel->setGeometry(QRect(30, 388, 344, 16));
        QFont font2;
        font2.setPointSize(9);
        copyrightLabel->setFont(font2);
        copyrightLabel->setAlignment(Qt::AlignCenter);
        layoutWidget = new QWidget(version);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 107, 571, 261));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        labelBranch = new QLabel(layoutWidget);
        labelBranch->setObjectName(QString::fromUtf8("labelBranch"));
        QFont font3;
        font3.setBold(true);
        font3.setWeight(75);
        labelBranch->setFont(font3);

        verticalLayout->addWidget(labelBranch);

        branchLabel = new QLabel(layoutWidget);
        branchLabel->setObjectName(QString::fromUtf8("branchLabel"));

        verticalLayout->addWidget(branchLabel);

        labelBuildTime = new QLabel(layoutWidget);
        labelBuildTime->setObjectName(QString::fromUtf8("labelBuildTime"));
        labelBuildTime->setFont(font3);

        verticalLayout->addWidget(labelBuildTime);

        buildTimeLabel = new QLabel(layoutWidget);
        buildTimeLabel->setObjectName(QString::fromUtf8("buildTimeLabel"));

        verticalLayout->addWidget(buildTimeLabel);

        commitLabel = new QLabel(layoutWidget);
        commitLabel->setObjectName(QString::fromUtf8("commitLabel"));

        verticalLayout->addWidget(commitLabel);


        retranslateUi(version);

        QMetaObject::connectSlotsByName(version);
    } // setupUi

    void retranslateUi(QWidget *version)
    {
        version->setWindowTitle(QApplication::translate("version", "\345\205\263\344\272\216 - MultiMediaTool", nullptr));
        appNameLabel->setText(QApplication::translate("version", "MultiMediaTool", nullptr));
        versionLabel->setStyleSheet(QApplication::translate("version", "color: #666;", nullptr));
        versionLabel->setText(QApplication::translate("version", "\347\211\210\346\234\254 1.0.0.67", nullptr));
        copyrightLabel->setStyleSheet(QApplication::translate("version", "color: #999;", nullptr));
        copyrightLabel->setText(QApplication::translate("version", "\302\251 2026 MultiMediaTool. All rights reserved.", nullptr));
        labelBranch->setText(QApplication::translate("version", "Git \345\210\206\346\224\257:", nullptr));
        branchLabel->setStyleSheet(QApplication::translate("version", "color: #333;", nullptr));
        branchLabel->setText(QApplication::translate("version", "main", nullptr));
        labelBuildTime->setText(QApplication::translate("version", "\346\236\204\345\273\272\346\227\266\351\227\264:", nullptr));
        buildTimeLabel->setStyleSheet(QApplication::translate("version", "color: #333;", nullptr));
        buildTimeLabel->setText(QApplication::translate("version", "2026-02-15 16:33:42", nullptr));
        commitLabel->setStyleSheet(QApplication::translate("version", "color: #333;", nullptr));
        commitLabel->setText(QApplication::translate("version", "66d972d", nullptr));
    } // retranslateUi

};

namespace Ui {
    class version: public Ui_version {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MVERSION_H
