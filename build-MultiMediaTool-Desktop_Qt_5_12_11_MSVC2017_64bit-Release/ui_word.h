/********************************************************************************
** Form generated from reading UI file 'word.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WORD_H
#define UI_WORD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_word
{
public:
    QVBoxLayout *mainVerticalLayout;
    QHBoxLayout *importLayout;
    QLineEdit *lineEdit_Path;
    QPushButton *btnImport;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QComboBox *comboBoxSrc;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QComboBox *comboBoxDst;
    QPushButton *pushButton;
    QListWidget *listWidget;

    void setupUi(QWidget *word)
    {
        if (word->objectName().isEmpty())
            word->setObjectName(QString::fromUtf8("word"));
        word->resize(612, 456);
        mainVerticalLayout = new QVBoxLayout(word);
        mainVerticalLayout->setSpacing(15);
        mainVerticalLayout->setObjectName(QString::fromUtf8("mainVerticalLayout"));
        mainVerticalLayout->setContentsMargins(20, 20, 20, 20);
        importLayout = new QHBoxLayout();
        importLayout->setObjectName(QString::fromUtf8("importLayout"));
        lineEdit_Path = new QLineEdit(word);
        lineEdit_Path->setObjectName(QString::fromUtf8("lineEdit_Path"));
        lineEdit_Path->setReadOnly(true);

        importLayout->addWidget(lineEdit_Path);

        btnImport = new QPushButton(word);
        btnImport->setObjectName(QString::fromUtf8("btnImport"));

        importLayout->addWidget(btnImport);


        mainVerticalLayout->addLayout(importLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(word);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        comboBoxSrc = new QComboBox(word);
        comboBoxSrc->setObjectName(QString::fromUtf8("comboBoxSrc"));

        verticalLayout->addWidget(comboBoxSrc);


        horizontalLayout->addLayout(verticalLayout);

        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_2 = new QLabel(word);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_2->addWidget(label_2);

        comboBoxDst = new QComboBox(word);
        comboBoxDst->setObjectName(QString::fromUtf8("comboBoxDst"));

        verticalLayout_2->addWidget(comboBoxDst);


        horizontalLayout->addLayout(verticalLayout_2);


        mainVerticalLayout->addLayout(horizontalLayout);

        pushButton = new QPushButton(word);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMinimumSize(QSize(0, 40));

        mainVerticalLayout->addWidget(pushButton);

        listWidget = new QListWidget(word);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        mainVerticalLayout->addWidget(listWidget);


        retranslateUi(word);

        QMetaObject::connectSlotsByName(word);
    } // setupUi

    void retranslateUi(QWidget *word)
    {
        word->setWindowTitle(QApplication::translate("word", "Form", nullptr));
        lineEdit_Path->setPlaceholderText(QApplication::translate("word", "\350\257\267\351\200\211\346\213\251\346\210\226\346\213\226\345\205\245\350\246\201\350\275\254\346\215\242\347\232\204\346\226\207\344\273\266...", nullptr));
        btnImport->setText(QApplication::translate("word", "\345\257\274\345\205\245\346\226\207\344\273\266", nullptr));
        label->setText(QApplication::translate("word", "\346\272\220\347\261\273\345\236\213", nullptr));
        label_2->setText(QApplication::translate("word", "\347\233\256\346\240\207\347\261\273\345\236\213", nullptr));
        pushButton->setText(QApplication::translate("word", "\345\274\200\345\247\213\350\275\254\346\215\242", nullptr));
    } // retranslateUi

};

namespace Ui {
    class word: public Ui_word {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WORD_H
