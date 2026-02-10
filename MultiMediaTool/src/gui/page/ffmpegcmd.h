#ifndef FFMPEGCMD_H
#define FFMPEGCMD_H

#include <QWidget>
#include <QProcess>
#include <QStringList>
#include <QTimer>

namespace Ui {
class ffmpegCmd;
}

class ffmpegCmd : public QWidget
{
    Q_OBJECT

public:
    explicit ffmpegCmd(QWidget *parent = nullptr);
    ~ffmpegCmd();

private slots:
    void on_btn_Run_clicked();
    void on_btn_Stop_clicked();
    void on_btn_ClearHistory_clicked();
    void onProcessOutputReady();
    void onProcessErrorReady();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onHistoryItemDoubleClicked(class QListWidgetItem *item);
    
    // AI助手相关槽函数
    void on_btn_AISend_clicked();
    void on_btn_AIClear_clicked();
    void on_btn_AISettings_clicked();
    void on_lineEdit_AIInput_returnPressed();
    void on_groupBox_AIAssistant_toggled(bool enabled);

private:
    void initPresets();
    void loadHistory();
    void saveHistory();
    void appendOutput(const QString &text, bool isError = false);
    
    // AI助手相关方法
    void initAIAssistant();
    void cleanupAIAssistant();
    void sendAIMessage(const QString &message);
    void appendAIChat(const QString &message, bool isUser = false);
    void updateAIStatus(const QString &status, const QString &color = "#4CAF50");
    void loadAISettings();
    void saveAISettings();
    void onAIResponseReceived(const QString &response);

    Ui::ffmpegCmd *ui;
    QProcess *m_process;
    QStringList m_history;
    const int m_maxHistory = 50;
    
    // AI助手相关成员
    void* m_aiChat = nullptr;
    QString m_apiKey;
    bool m_aiEnabled = true;
    QTimer* m_aiTimer;
};

#endif // FFMPEGCMD_H
