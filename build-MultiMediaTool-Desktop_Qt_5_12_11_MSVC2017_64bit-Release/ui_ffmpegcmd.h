/********************************************************************************
** Form generated from reading UI file 'ffmpegcmd.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FFMPEGCMD_H
#define UI_FFMPEGCMD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ffmpegCmd
{
public:
    QVBoxLayout *verticalLayout_Main;
    QHBoxLayout *horizontalLayout_Top;
    QGroupBox *groupBox_Presets;
    QVBoxLayout *verticalLayout_Presets;
    QListWidget *listWidget_History;
    QPushButton *btn_ClearHistory;
    QGroupBox *groupBox_AIAssistant;
    QVBoxLayout *verticalLayout_AI;
    QLabel *label_AIStatus;
    QPlainTextEdit *textEdit_AIChat;
    QLineEdit *lineEdit_AIInput;
    QHBoxLayout *horizontalLayout_AIButtons;
    QPushButton *btn_AISend;
    QPushButton *btn_AIClear;
    QPushButton *btn_AISettings;
    QVBoxLayout *verticalLayout_Command;
    QGroupBox *groupBox_Input;
    QVBoxLayout *verticalLayout_Input;
    QPlainTextEdit *textEdit_Command;
    QHBoxLayout *horizontalLayout_Buttons;
    QSpacerItem *horizontalSpacer;
    QPushButton *btn_Run;
    QPushButton *btn_Stop;
    QGroupBox *groupBox_Output;
    QVBoxLayout *verticalLayout_Output;
    QPlainTextEdit *textEdit_Output;

    void setupUi(QWidget *ffmpegCmd)
    {
        if (ffmpegCmd->objectName().isEmpty())
            ffmpegCmd->setObjectName(QString::fromUtf8("ffmpegCmd"));
        ffmpegCmd->resize(800, 600);
        verticalLayout_Main = new QVBoxLayout(ffmpegCmd);
        verticalLayout_Main->setObjectName(QString::fromUtf8("verticalLayout_Main"));
        horizontalLayout_Top = new QHBoxLayout();
        horizontalLayout_Top->setObjectName(QString::fromUtf8("horizontalLayout_Top"));
        groupBox_Presets = new QGroupBox(ffmpegCmd);
        groupBox_Presets->setObjectName(QString::fromUtf8("groupBox_Presets"));
        groupBox_Presets->setMaximumSize(QSize(250, 16777215));
        verticalLayout_Presets = new QVBoxLayout(groupBox_Presets);
        verticalLayout_Presets->setObjectName(QString::fromUtf8("verticalLayout_Presets"));
        listWidget_History = new QListWidget(groupBox_Presets);
        listWidget_History->setObjectName(QString::fromUtf8("listWidget_History"));

        verticalLayout_Presets->addWidget(listWidget_History);

        btn_ClearHistory = new QPushButton(groupBox_Presets);
        btn_ClearHistory->setObjectName(QString::fromUtf8("btn_ClearHistory"));

        verticalLayout_Presets->addWidget(btn_ClearHistory);


        horizontalLayout_Top->addWidget(groupBox_Presets);

        groupBox_AIAssistant = new QGroupBox(ffmpegCmd);
        groupBox_AIAssistant->setObjectName(QString::fromUtf8("groupBox_AIAssistant"));
        groupBox_AIAssistant->setMaximumSize(QSize(250, 16777215));
        groupBox_AIAssistant->setCheckable(true);
        groupBox_AIAssistant->setChecked(false);
        verticalLayout_AI = new QVBoxLayout(groupBox_AIAssistant);
        verticalLayout_AI->setObjectName(QString::fromUtf8("verticalLayout_AI"));
        label_AIStatus = new QLabel(groupBox_AIAssistant);
        label_AIStatus->setObjectName(QString::fromUtf8("label_AIStatus"));

        verticalLayout_AI->addWidget(label_AIStatus);

        textEdit_AIChat = new QPlainTextEdit(groupBox_AIAssistant);
        textEdit_AIChat->setObjectName(QString::fromUtf8("textEdit_AIChat"));
        textEdit_AIChat->setMaximumSize(QSize(16777215, 200));
        textEdit_AIChat->setReadOnly(true);

        verticalLayout_AI->addWidget(textEdit_AIChat);

        lineEdit_AIInput = new QLineEdit(groupBox_AIAssistant);
        lineEdit_AIInput->setObjectName(QString::fromUtf8("lineEdit_AIInput"));

        verticalLayout_AI->addWidget(lineEdit_AIInput);

        horizontalLayout_AIButtons = new QHBoxLayout();
        horizontalLayout_AIButtons->setObjectName(QString::fromUtf8("horizontalLayout_AIButtons"));
        btn_AISend = new QPushButton(groupBox_AIAssistant);
        btn_AISend->setObjectName(QString::fromUtf8("btn_AISend"));

        horizontalLayout_AIButtons->addWidget(btn_AISend);

        btn_AIClear = new QPushButton(groupBox_AIAssistant);
        btn_AIClear->setObjectName(QString::fromUtf8("btn_AIClear"));

        horizontalLayout_AIButtons->addWidget(btn_AIClear);

        btn_AISettings = new QPushButton(groupBox_AIAssistant);
        btn_AISettings->setObjectName(QString::fromUtf8("btn_AISettings"));

        horizontalLayout_AIButtons->addWidget(btn_AISettings);


        verticalLayout_AI->addLayout(horizontalLayout_AIButtons);


        horizontalLayout_Top->addWidget(groupBox_AIAssistant);

        verticalLayout_Command = new QVBoxLayout();
        verticalLayout_Command->setObjectName(QString::fromUtf8("verticalLayout_Command"));
        groupBox_Input = new QGroupBox(ffmpegCmd);
        groupBox_Input->setObjectName(QString::fromUtf8("groupBox_Input"));
        verticalLayout_Input = new QVBoxLayout(groupBox_Input);
        verticalLayout_Input->setObjectName(QString::fromUtf8("verticalLayout_Input"));
        textEdit_Command = new QPlainTextEdit(groupBox_Input);
        textEdit_Command->setObjectName(QString::fromUtf8("textEdit_Command"));

        verticalLayout_Input->addWidget(textEdit_Command);

        horizontalLayout_Buttons = new QHBoxLayout();
        horizontalLayout_Buttons->setObjectName(QString::fromUtf8("horizontalLayout_Buttons"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Buttons->addItem(horizontalSpacer);

        btn_Run = new QPushButton(groupBox_Input);
        btn_Run->setObjectName(QString::fromUtf8("btn_Run"));
        btn_Run->setMinimumSize(QSize(100, 40));

        horizontalLayout_Buttons->addWidget(btn_Run);

        btn_Stop = new QPushButton(groupBox_Input);
        btn_Stop->setObjectName(QString::fromUtf8("btn_Stop"));
        btn_Stop->setMinimumSize(QSize(100, 40));

        horizontalLayout_Buttons->addWidget(btn_Stop);


        verticalLayout_Input->addLayout(horizontalLayout_Buttons);


        verticalLayout_Command->addWidget(groupBox_Input);

        groupBox_Output = new QGroupBox(ffmpegCmd);
        groupBox_Output->setObjectName(QString::fromUtf8("groupBox_Output"));
        verticalLayout_Output = new QVBoxLayout(groupBox_Output);
        verticalLayout_Output->setObjectName(QString::fromUtf8("verticalLayout_Output"));
        textEdit_Output = new QPlainTextEdit(groupBox_Output);
        textEdit_Output->setObjectName(QString::fromUtf8("textEdit_Output"));
        textEdit_Output->setReadOnly(true);
        textEdit_Output->setBackgroundVisible(false);

        verticalLayout_Output->addWidget(textEdit_Output);


        verticalLayout_Command->addWidget(groupBox_Output);


        horizontalLayout_Top->addLayout(verticalLayout_Command);


        verticalLayout_Main->addLayout(horizontalLayout_Top);


        retranslateUi(ffmpegCmd);

        QMetaObject::connectSlotsByName(ffmpegCmd);
    } // setupUi

    void retranslateUi(QWidget *ffmpegCmd)
    {
        groupBox_Presets->setTitle(QApplication::translate("ffmpegCmd", "\345\221\275\344\273\244\350\256\260\345\275\225\344\270\216\351\242\204\350\256\276", nullptr));
        btn_ClearHistory->setText(QApplication::translate("ffmpegCmd", "\346\270\205\351\231\244\345\216\206\345\217\262", nullptr));
        groupBox_AIAssistant->setTitle(QApplication::translate("ffmpegCmd", "AI \346\231\272\350\203\275\345\212\251\346\211\213", nullptr));
        label_AIStatus->setText(QApplication::translate("ffmpegCmd", "AI\345\212\251\346\211\213\345\267\262\345\260\261\347\273\252", nullptr));
        label_AIStatus->setStyleSheet(QApplication::translate("ffmpegCmd", "color: #4CAF50; font-weight: bold;", nullptr));
        textEdit_AIChat->setPlaceholderText(QApplication::translate("ffmpegCmd", "AI\345\233\236\345\244\215\345\260\206\346\230\276\347\244\272\345\234\250\350\277\231\351\207\214...", nullptr));
        lineEdit_AIInput->setPlaceholderText(QApplication::translate("ffmpegCmd", "\345\220\221AI\345\212\251\346\211\213\346\217\220\351\227\256...", nullptr));
        btn_AISend->setText(QApplication::translate("ffmpegCmd", "\345\217\221\351\200\201", nullptr));
        btn_AIClear->setText(QApplication::translate("ffmpegCmd", "\346\270\205\347\251\272", nullptr));
        btn_AISettings->setText(QApplication::translate("ffmpegCmd", "\350\256\276\347\275\256", nullptr));
        groupBox_Input->setTitle(QApplication::translate("ffmpegCmd", "FFmpeg \345\221\275\344\273\244\350\276\223\345\205\245", nullptr));
        textEdit_Command->setPlaceholderText(QApplication::translate("ffmpegCmd", "\345\234\250\346\255\244\350\276\223\345\205\245 ffmpeg \345\221\275\344\273\244\357\274\214\344\276\213\345\246\202: ffmpeg -i input.mp4 output.avi", nullptr));
        btn_Run->setText(QApplication::translate("ffmpegCmd", "\346\211\247\350\241\214\345\221\275\344\273\244", nullptr));
        btn_Stop->setText(QApplication::translate("ffmpegCmd", "\345\201\234\346\255\242\346\211\247\350\241\214", nullptr));
        groupBox_Output->setTitle(QApplication::translate("ffmpegCmd", "\346\211\247\350\241\214\350\276\223\345\207\272", nullptr));
        Q_UNUSED(ffmpegCmd);
    } // retranslateUi

};

namespace Ui {
    class ffmpegCmd: public Ui_ffmpegCmd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FFMPEGCMD_H
