#ifndef FFMPEGCMD_H
#define FFMPEGCMD_H

#include <QWidget>
#include <QProcess>
#include <QStringList>

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

private:
    void initPresets();
    void loadHistory();
    void saveHistory();
    void appendOutput(const QString &text, bool isError = false);

    Ui::ffmpegCmd *ui;
    QProcess *m_process;
    QStringList m_history;
    const int m_maxHistory = 50;
};

#endif // FFMPEGCMD_H
