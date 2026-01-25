#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QMainWindow>
#include "mytitlebar.h"

class basewindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit basewindow(QWidget *parent = nullptr);
    ~basewindow() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void loadStyleSheet(const QString &sheetName);

private slots:
    void onButtonMinClicked();
    void onButtonRestoreClicked();
    void onButtonMaxClicked();
    void onButtonCloseClicked();

private:
    void initTitleBar();

    // 成员变量声明
    myTitleBar *m_titleBar = nullptr;
};

#endif // BASEWINDOW_H
