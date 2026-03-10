#include "reverse.h"
#include "ui_reverse.h"
#include <cstring>

reverse::reverse(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::reverse)
{
    ui->setupUi(this);
}

reverse::~reverse()
{
    delete ui;
}

param reverse::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    p.iparam1 = 0;                          // X坐标
    p.iparam2 = 0;                          // Y坐标
    p.dparam1 = 0;                // 字体大小
    p.iparam3 = 0;                          // R
    p.iparam4 = 0;                          // G
    p.iparam5 = 0;                          // B
    p.iparam3 = 0;                  // 粗细
    std::string text = "";
    strncpy(p.arr, text.c_str(), sizeof(p.arr) - 1);             // 水印文字
    p.arr[sizeof(p.arr) - 1] = '\0';
    return p;
}
