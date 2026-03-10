/********************************************************************************
** Form generated from reading UI file 'videopage.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOPAGE_H
#define UI_VIDEOPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_videoPage
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QLabel *recordLabel;
    QComboBox *recordComboBox;
    QPushButton *begin;
    QTableWidget *tableWidget;
    QPushButton *pushButton;
    QPushButton *flashbutton;
    QPushButton *import_2;

    void setupUi(QWidget *videoPage)
    {
        if (videoPage->objectName().isEmpty())
            videoPage->setObjectName(QString::fromUtf8("videoPage"));
        videoPage->resize(832, 574);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(videoPage->sizePolicy().hasHeightForWidth());
        videoPage->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(videoPage);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        recordLabel = new QLabel(videoPage);
        recordLabel->setObjectName(QString::fromUtf8("recordLabel"));

        verticalLayout->addWidget(recordLabel);

        recordComboBox = new QComboBox(videoPage);
        recordComboBox->setObjectName(QString::fromUtf8("recordComboBox"));

        verticalLayout->addWidget(recordComboBox);


        horizontalLayout->addLayout(verticalLayout);

        begin = new QPushButton(videoPage);
        begin->setObjectName(QString::fromUtf8("begin"));

        horizontalLayout->addWidget(begin);


        verticalLayout_2->addLayout(horizontalLayout);

        tableWidget = new QTableWidget(videoPage);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));

        verticalLayout_2->addWidget(tableWidget);

        pushButton = new QPushButton(videoPage);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout_2->addWidget(pushButton);

        flashbutton = new QPushButton(videoPage);
        flashbutton->setObjectName(QString::fromUtf8("flashbutton"));

        verticalLayout_2->addWidget(flashbutton);

        import_2 = new QPushButton(videoPage);
        import_2->setObjectName(QString::fromUtf8("import_2"));

        verticalLayout_2->addWidget(import_2);


        verticalLayout_3->addLayout(verticalLayout_2);


        retranslateUi(videoPage);

        QMetaObject::connectSlotsByName(videoPage);
    } // setupUi

    void retranslateUi(QWidget *videoPage)
    {
        videoPage->setWindowTitle(QApplication::translate("videoPage", "Form", nullptr));
        recordLabel->setText(QApplication::translate("videoPage", "\351\200\211\346\213\251\345\275\225\345\210\266\345\212\237\350\203\275", nullptr));
        begin->setText(QApplication::translate("videoPage", "\345\274\200\345\247\213", nullptr));
        pushButton->setText(QApplication::translate("videoPage", "\350\247\206\351\242\221\346\223\215\344\275\234", nullptr));
        flashbutton->setText(QApplication::translate("videoPage", "\345\210\267\346\226\260", nullptr));
        import_2->setText(QApplication::translate("videoPage", "\345\257\274\345\205\245", nullptr));
    } // retranslateUi

};

namespace Ui {
    class videoPage: public Ui_videoPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOPAGE_H
