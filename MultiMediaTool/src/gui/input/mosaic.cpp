#include "mosaic.h"
#include "ui_mosaic.h"
#include <cstring>

mosaic::mosaic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mosaic)
{
    ui->setupUi(this);
}

mosaic::~mosaic()
{
    delete ui;
}

param mosaic::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    p.iparam1 = ui->spinBox_x->value();      // X坐标
    p.iparam2 = ui->spinBox_y->value();      // Y坐标
    p.iparam3 = ui->spinBox_width->value();  // 宽度
    p.iparam4 = ui->spinBox_height->value(); // 高度
    p.iparam5 = ui->spinBox_blockSize->value(); // 马赛克块大小
    return p;
}
