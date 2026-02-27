#ifndef VIDEOPAGE_H
#define VIDEOPAGE_H

#include <QWidget>
#include <QString>
#include "src/gui/page/concat.h"
#include "OpenCVFFMpegTools.h"
#include "src/base/pagebase.h"

namespace Ui {
class videoPage;
}

class MyIPCMgr;

class videoPage : public QWidget
{
    Q_OBJECT

public:
    explicit videoPage(QWidget *parent = nullptr);
    ~videoPage();
    bool init();
    bool initableWidget();
    void setIPCMgr(MyIPCMgr *mgr) { m_ipcMgr = mgr; }

private slots:
    void on_begin_clicked();
    void on_flashbutton_clicked();
    void on_import_2_clicked();
    void on_pushButton_clicked();
    void on_recordComboBox_currentIndexChanged(int index);

private:
    concat *mconcat;
    QProcess *process;
    void *worker = NULL;
    Ui::videoPage *ui;
    MyIPCMgr *m_ipcMgr = nullptr;
};


#endif // VIDEOPAGE_H
