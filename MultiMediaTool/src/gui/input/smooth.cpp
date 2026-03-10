#include "smooth.h"
#include "ui_smooth.h"
#include <cstring>

smooth::smooth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::smooth)
{
    ui->setupUi(this);
}

smooth::~smooth()
{
    delete ui;
}

param smooth::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    return p;
}
