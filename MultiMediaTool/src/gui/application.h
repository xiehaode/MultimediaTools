#ifndef APPLICATION_H
#define APPLICATION_H

#include "basewindow.h"
#include "menu.h"
#include "src/gui/page/videopage.h"
#include "src/gui/page/ffmpegclientwidget.h"
#include <QStackedWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>

class MyIPCMgr;

class MainWindow : public basewindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    MyIPCMgr* getIPCMgr() const { return m_ipcMgr; }

private slots:
    void onButtonClicked(int buttonId);

private:
    bool createleftNavigationBar();
    QSplitter* createVerticalSplitter(QWidget *parent, QWidget *content1, QWidget *content2,
                                      Qt::Orientation orientation, int size1, int size2);

    // 成员变量声明
    MyIPCMgr *m_ipcMgr = nullptr;
    QStackedWidget *qStackedWidget = nullptr;
    Menu *sidebarWidget = nullptr;
    QWidget *content = nullptr;
};

#endif // APPLICATION_H
