#include "ffmpegcmd.h"
#include "ui_ffmpegcmd.h"
#include "src/utils/encodinghelper.h"

#include "src/utils/encodinghelper.h"
#include <QSettings>
#include <QDateTime>
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

ffmpegCmd::ffmpegCmd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ffmpegCmd),
    m_process(new QProcess(this))
{
    ui->setupUi(this);

    // 设置输出框样式
    ui->textEdit_Output->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: 'Consolas', 'Monaco', monospace;");

    // 连接进程信号
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ffmpegCmd::onProcessOutputReady);
    connect(m_process, &QProcess::readyReadStandardError, this, &ffmpegCmd::onProcessErrorReady);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ffmpegCmd::onProcessFinished);

    // 连接列表双击信号
    connect(ui->listWidget_History, &QListWidget::itemDoubleClicked, this, &ffmpegCmd::onHistoryItemDoubleClicked);

    initPresets();
    loadHistory();
}

ffmpegCmd::~ffmpegCmd()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }
    delete ui;
}

void ffmpegCmd::initPresets()
{
    // 添加一些基础命令
    QStringList presets = {
        "ffmpeg -i input.mp4 output.mp3",
        "ffmpeg -i input.mp4 -vn -acodec copy output.m4a",
        "ffmpeg -i input.mp4 -vf scale=1280:720 output_720p.mp4",
        "ffmpeg -i input.mp4 -ss 00:00:10 -t 00:00:20 -c copy output_clip.mp4",
        "ffmpeg -i input.mp4 -vf \"transpose=1\" output_rotated.mp4",
        "ffmpeg -i \"concat:input1.ts|input2.ts\" -c copy output.mp4"
    };

    for (const QString &cmd : presets) {
        QListWidgetItem *item = new QListWidgetItem(cmd);
        item->setToolTip("双击以载入命令");
        ui->listWidget_History->addItem(item);
    }

    // 分隔线
    QListWidgetItem *separator = new QListWidgetItem("---------- 历史记录 ----------");
    separator->setFlags(Qt::NoItemFlags);
    ui->listWidget_History->addItem(separator);
}

void ffmpegCmd::loadHistory()
{
    QSettings settings("MultiMediaTool", "FFmpegHistory");
    m_history = settings.value("commands").toStringList();
    for (const QString &cmd : m_history) {
        ui->listWidget_History->addItem(cmd);
    }
}

void ffmpegCmd::saveHistory()
{
    QSettings settings("MultiMediaTool", "FFmpegHistory");
    settings.setValue("commands", m_history);
}

void ffmpegCmd::on_btn_Run_clicked()
{
    QString fullCmd = ui->textEdit_Command->toPlainText().trimmed();
    if (fullCmd.isEmpty()) return;

    if (m_process->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "警告", "已有命令正在执行中！");
        return;
    }

    // 更新历史记录
    if (!m_history.contains(fullCmd)) {
        m_history.prepend(fullCmd);
        if (m_history.size() > m_maxHistory) m_history.removeLast();
        
        // 刷新列表显示
        ui->listWidget_History->clear();
        initPresets(); 
        // 将当前历史记录也重新添加
        for (const QString &hCmd : m_history) {
            ui->listWidget_History->addItem(hCmd);
        }
        saveHistory();
    }


    ui->textEdit_Output->clear();
    appendOutput("开始执行: " + fullCmd + "\n");
    
    ui->btn_Run->setEnabled(false);
    ui->btn_Stop->setEnabled(true);

    // 简单解析命令（注意：这里可能需要更复杂的解析以支持引号）
    // 为了简单起见，我们直接调用 cmd /c 在 Windows 上执行，或者直接解析
#ifdef Q_OS_WIN
    m_process->start("cmd", QStringList() << "/c" << fullCmd);
#else
    m_process->start("/bin/sh", QStringList() << "-c" << fullCmd);
#endif
}

void ffmpegCmd::on_btn_Stop_clicked()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
        appendOutput("\n命令已被用户终止。\n", true);
    }
}

void ffmpegCmd::on_btn_ClearHistory_clicked()
{
    m_history.clear();
    saveHistory();
    ui->listWidget_History->clear();
    initPresets();
}

void ffmpegCmd::onProcessOutputReady()
{
    QByteArray data = m_process->readAllStandardOutput();
    appendOutput(GBK2QString(data)); 
}

void ffmpegCmd::onProcessErrorReady()
{
    // FFmpeg 的进度信息通常输出到 stderr
    QByteArray data = m_process->readAllStandardError();
    appendOutput(GBK2QString(data), true);
}


void ffmpegCmd::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->btn_Run->setEnabled(true);
    ui->btn_Stop->setEnabled(false);
    
    QString statusStr = (exitStatus == QProcess::NormalExit && exitCode == 0) ? "成功" : "失败";
    appendOutput(QString("\n执行完成! 状态: %1 (退出代码: %2)\n").arg(statusStr).arg(exitCode));
}

void ffmpegCmd::onHistoryItemDoubleClicked(QListWidgetItem *item)
{
    if (item->flags() & Qt::ItemIsEnabled) {
        ui->textEdit_Command->setPlainText(item->text());
    }
}

void ffmpegCmd::appendOutput(const QString &text, bool isError)
{
    QTextCursor cursor = ui->textEdit_Output->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    if (isError) {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#ff5555")); // 浅红
        cursor.setCharFormat(fmt);
    } else {
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#d4d4d4"));
        cursor.setCharFormat(fmt);
    }
    
    cursor.insertText(text);
    ui->textEdit_Output->setTextCursor(cursor);
    ui->textEdit_Output->verticalScrollBar()->setValue(ui->textEdit_Output->verticalScrollBar()->maximum());
}
