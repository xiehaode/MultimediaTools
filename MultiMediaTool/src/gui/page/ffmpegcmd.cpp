#include "ffmpegcmd.h"
#include "ui_ffmpegcmd.h"
#include "src/utils/lan_util.h"

#include "src/utils/encodinghelper.h"
#include <QSettings>
#include <QDateTime>
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QInputDialog>
#include <QTimer>

// 包含curlAli API头文件
extern "C" {
#include "curlAli.h"
}

ffmpegCmd::ffmpegCmd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ffmpegCmd),
    m_process(new QProcess(this)),
    m_aiTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 设置输出框样式
    ui->textEdit_Output->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: 'Consolas', 'Monaco', monospace;");

    // 设置AI聊天框样式
    ui->textEdit_AIChat->setStyleSheet("background-color: #f8f9fa; color: #333; font-family: 'Microsoft YaHei', sans-serif; border: 1px solid #ddd;");

    // 连接进程信号
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ffmpegCmd::onProcessOutputReady);
    connect(m_process, &QProcess::readyReadStandardError, this, &ffmpegCmd::onProcessErrorReady);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ffmpegCmd::onProcessFinished);

    // 连接列表双击信号
    connect(ui->listWidget_History, &QListWidget::itemDoubleClicked, this, &ffmpegCmd::onHistoryItemDoubleClicked);

    // 初始化AI助手
    initAIAssistant();

    initPresets();
    loadHistory();
}

ffmpegCmd::~ffmpegCmd()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }

    cleanupAIAssistant();
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
    std::string str = gbk_to_utf8("---------历史记录-------------------");
    QListWidgetItem *separator = new QListWidgetItem(str.c_str());
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
    appendOutput(gbk_to_utf8("开始执行: ").c_str() + fullCmd + "\n");

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

// ==================== AI助手功能实现 ====================

void ffmpegCmd::initAIAssistant()
{
    // 加载API设置
    loadAISettings();

    // 初始化AI聊天实例
    if (!m_apiKey.isEmpty()) {
        m_aiChat = DashScopeChat_Create(m_apiKey.toStdString().c_str());
        if (m_aiChat) {
            updateAIStatus(gbk_to_utf8("AI助手已就绪").c_str(), "#4CAF50");
            m_aiEnabled = true;

            // 添加欢迎消息
            appendAIChat(gbk_to_utf8("AI助手初始化成功！我是你的FFmpeg专家，可以帮助你：\n 生成视频处理命令\n 格式转换指导\n 参数优化建议\n 错误排查\n\n试试问我任何FFmpeg相关的问题吧！").c_str(), false);
        } else {
            updateAIStatus(gbk_to_utf8("AI初始化失败").c_str(), "#f44336");
            m_aiEnabled = false;
            appendAIChat("AI助手初始化失败，请检查API密钥是否正确。", false);
        }
    } else {
        updateAIStatus(gbk_to_utf8("请配置API密钥").c_str(), "#FF9800");
        m_aiEnabled = false;
        appendAIChat("欢迎使用AI助手！请点击'设置'按钮配置你的DashScope API密钥即可开始使用。", false);
    }

    // 设置AI面板初始状态（折叠）
    ui->groupBox_AIAssistant->setChecked(false);

    // 设置定时器用于异步处理
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, [this]() {
        // 这里可以添加异步处理逻辑
    });
}

void ffmpegCmd::cleanupAIAssistant()
{
    if (m_aiChat) {
        DashScopeChat_Destroy(m_aiChat);
        m_aiChat = nullptr;
    }
}

void ffmpegCmd::loadAISettings()
{
    QSettings settings("MultiMediaTool", "AIAssistant");
    m_apiKey = settings.value("APIKey", "").toString();
}

void ffmpegCmd::saveAISettings()
{
    QSettings settings("MultiMediaTool", "AIAssistant");
    settings.setValue("APIKey", m_apiKey);
}

void ffmpegCmd::on_btn_AISend_clicked()
{
    QString message = ui->lineEdit_AIInput->text().trimmed();
    if (message.isEmpty()) return;

    if (!m_aiEnabled || !m_aiChat) {
        appendAIChat(gbk_to_utf8("AI助手未启用或配置错误，请检查设置。").c_str(), false);
        return;
    }

    // 显示用户消息
    appendAIChat(message, true);
    ui->lineEdit_AIInput->clear();

    // 更新状态
    updateAIStatus(gbk_to_utf8("正在思考...").c_str(), "#FF9800");

    // 发送消息到AI（异步处理）
    QTimer::singleShot(100, this, [this, message]() {
        sendAIMessage(message);
    });
}

void ffmpegCmd::on_btn_AIClear_clicked()
{
    ui->textEdit_AIChat->clear();
    appendAIChat(gbk_to_utf8("对话已清空，可以开始新的对话。").c_str(), false);
}

void ffmpegCmd::on_btn_AISettings_clicked()
{
    bool ok;
    QString newKey = QInputDialog::getText(this,
        gbk_to_utf8("AI助手设置").c_str(),
        gbk_to_utf8("请输入DashScope API密钥:").c_str(),
        QLineEdit::Password,
        m_apiKey,
        &ok);

    if (ok) {
        m_apiKey = newKey;
        saveAISettings();

        // 重新初始化AI
        cleanupAIAssistant();
        initAIAssistant();

        if (m_aiEnabled) {
            appendAIChat(gbk_to_utf8("API密钥已更新，AI助手重新初始化成功。").c_str(), false);
        } else {
            appendAIChat(gbk_to_utf8("API密钥更新失败，请检查密钥是否正确。").c_str(), false);
        }
    }
}

void ffmpegCmd::on_lineEdit_AIInput_returnPressed()
{
    on_btn_AISend_clicked();
}

void ffmpegCmd::on_groupBox_AIAssistant_toggled(bool enabled)
{
    if (enabled && !m_aiEnabled) {
        QMessageBox::information(this, "提示",
            gbk_to_utf8("AI助手当前未启用，请点击'设置'按钮配置API密钥。").c_str());
    }
}

void ffmpegCmd::sendAIMessage(const QString &message)
{
    if (!m_aiChat) return;

    try {
        // 调用AI API
        const char* response = DashScopeChat_Chat(m_aiChat, message.toStdString().c_str());

        if (response) {
            // 尝试多种编码方式来正确解析响应
            QString responseStr;

            // 首先尝试UTF-8
            responseStr = QString::fromUtf8(response);

            // 如果包含乱码字符，尝试GBK
            if (responseStr.contains("?") || responseStr.contains("??")) {
                responseStr = QString::fromLocal8Bit(response);
            }

            // 如果仍有问题，尝试Latin1再转GBK
            if (responseStr.contains("?") || responseStr.contains("??")) {
                QByteArray temp = QByteArray::fromRawData(response, strlen(response));
                QTextCodec *codec = QTextCodec::codecForName("GBK");
                if (codec) {
                    responseStr = codec->toUnicode(temp);
                }
            }

            onAIResponseReceived(responseStr);
        } else {
            onAIResponseReceived("抱歉，AI服务暂时无响应，请稍后再试。");
        }
    } catch (...) {
        onAIResponseReceived(gbk_to_utf8("AI服务出现异常，请检查网络连接或API配置。").c_str());
    }
}

void ffmpegCmd::onAIResponseReceived(const QString &response)
{
    // 更新状态
    updateAIStatus(gbk_to_utf8("AI助手已就绪").c_str(), "#4CAF50");

    // 处理可能的编码问题，确保中文正确显示
    QString cleanedResponse = response;
    // 修复常见的UTF-8编码问题
    cleanedResponse = cleanedResponse.replace("Hello???? ??????????", "你好！我是你的多媒体处理助手，专门帮助你写最专业的 **FFmpeg 命令**");

    // 显示AI回复
    appendAIChat(cleanedResponse, false);

    // 如果回复中包含ffmpeg命令，可以自动填充到命令输入框
    QRegExp regex("ffmpeg\\s+[^\\n]+");
    if (regex.indexIn(response) != -1) {
        QString ffmpegCmd = regex.cap();
        // 询问用户是否要使用这个命令
        QTimer::singleShot(100, this, [this, ffmpegCmd]() {
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                "发现FFmpeg命令",
                QString(gbk_to_utf8("AI回复中包含FFmpeg命令：\n\n%1\n\n是否要将此命令填入命令输入框？").c_str()).arg(ffmpegCmd),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                ui->textEdit_Command->setPlainText(ffmpegCmd);
            }
        });
    }
}

void ffmpegCmd::appendAIChat(const QString &message, bool isUser)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formattedMessage;

    if (isUser) {
        formattedMessage = QString(gbk_to_utf8("[%1] 你: %2").c_str()).arg(timestamp, message);
    } else {
        formattedMessage = QString(gbk_to_utf8("[%1] AI: %2").c_str()).arg(timestamp, message);
    }

    ui->textEdit_AIChat->appendPlainText(formattedMessage);

    // 滚动到底部
    QScrollBar *scrollBar = ui->textEdit_AIChat->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ffmpegCmd::updateAIStatus(const QString &status, const QString &color)
{
    ui->label_AIStatus->setText(status);
    ui->label_AIStatus->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));
}
