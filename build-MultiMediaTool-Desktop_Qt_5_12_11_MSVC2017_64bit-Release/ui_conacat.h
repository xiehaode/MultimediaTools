/********************************************************************************
** Form generated from reading UI file 'conacat.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONACAT_H
#define UI_CONACAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_conacat
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btn_Split;
    QPushButton *btn_Refresh;
    QPushButton *btn_Merge;
    QPushButton *btn_Resize;

    void setupUi(QWidget *conacat)
    {
        if (conacat->objectName().isEmpty())
            conacat->setObjectName(QString::fromUtf8("conacat"));
        conacat->resize(891, 630);
        widget = new QWidget(conacat);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(50, 10, 611, 531));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tableWidget = new QTableWidget(widget);
        if (tableWidget->columnCount() < 3)
            tableWidget->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayout->addWidget(tableWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        btn_Split = new QPushButton(widget);
        btn_Split->setObjectName(QString::fromUtf8("btn_Split"));
        btn_Split->setEnabled(false);

        horizontalLayout->addWidget(btn_Split);

        btn_Refresh = new QPushButton(widget);
        btn_Refresh->setObjectName(QString::fromUtf8("btn_Refresh"));

        horizontalLayout->addWidget(btn_Refresh);

        btn_Merge = new QPushButton(widget);
        btn_Merge->setObjectName(QString::fromUtf8("btn_Merge"));
        btn_Merge->setEnabled(false);

        horizontalLayout->addWidget(btn_Merge);

        btn_Resize = new QPushButton(widget);
        btn_Resize->setObjectName(QString::fromUtf8("btn_Resize"));
        btn_Resize->setEnabled(false);

        horizontalLayout->addWidget(btn_Resize);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(conacat);

        QMetaObject::connectSlotsByName(conacat);
    } // setupUi

    void retranslateUi(QWidget *conacat)
    {
        conacat->setWindowTitle(QApplication::translate("conacat", "\350\247\206\351\242\221\345\220\210\345\271\266/\345\210\206\345\211\262/\350\260\203\346\225\264\345\244\247\345\260\217", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("conacat", "\351\200\211\346\213\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("conacat", "\346\226\207\344\273\266\345\220\215", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("conacat", "\350\267\257\345\276\204", nullptr));
        btn_Split->setText(QApplication::translate("conacat", "\345\210\206\345\211\262\350\247\206\351\242\221", nullptr));
        btn_Refresh->setText(QApplication::translate("conacat", "\345\210\267\346\226\260", nullptr));
        btn_Merge->setText(QApplication::translate("conacat", "\345\220\210\345\271\266\350\247\206\351\242\221", nullptr));
        btn_Resize->setText(QApplication::translate("conacat", "\350\260\203\346\225\264\345\244\247\345\260\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class conacat: public Ui_conacat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONACAT_H
