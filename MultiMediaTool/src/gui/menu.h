#ifndef MENU_H
#define MENU_H


#include "src/utils/encodinghelper.h"
#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QFont>
#include <QIcon>

class Menu : public QWidget
{
    Q_OBJECT
public:
    explicit Menu(QWidget *parent = nullptr);

signals:
    // 侧边栏按钮点击信号
    void buttonClicked(int id);
    // 最小化按钮点击信号
    void minimizeClicked();

private slots:
    void onButtonClicked(int id);
    void onMinimizeButtonClicked();

private:
    void init();
    void setMenu(bool isMini);
    // 通用按钮设置函数
    void setPushButton(QPushButton *button, const QString& iconUrl, const QString& text);
    void setPushButton(QPushButton *button, const QString& text);

    // 成员变量声明（修复未初始化问题）

    QPushButton *newPage1BtnVideo;
    QPushButton *newPage2BtnWord;
    QPushButton *newPage3BtnVideoEffact;
    QPushButton *newPage4BtnPicture;
    QPushButton *newPage5BtnFFmpeg;
    QPushButton *newPage6BtnFFmpegClient;
    QButtonGroup buttonGroup;

    bool isMinimized = false; // 侧边栏最小化状态标记
};

#endif // MENU_H
