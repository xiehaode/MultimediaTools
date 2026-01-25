#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "mytitlebar.h"

class basewindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit basewindow(QWidget *parent = nullptr);
    ~basewindow() override;

    //获取内容区的窗口
    QWidget* getContentWidget();
protected:
    void paintEvent(QPaintEvent* event) override;
    void loadStyleSheet(const QString &sheetName);

private slots:
    void onButtonMinClicked();
    void onButtonRestoreClicked();
    void onButtonMaxClicked();
    void onButtonCloseClicked();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initTitleBar();

    // 成员变量声明
    myTitleBar *m_titleBar = nullptr;
    // 内容区专用部件
    QWidget *m_contentWidget;
};

#endif // BASEWINDOW_H
