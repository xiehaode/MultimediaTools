#include "white2.h"
#include "ui_white2.h"
#include <cstring>

white2::white2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::white2)
{
    ui->setupUi(this);
}

white2::~white2()
{
    delete ui;
}

param white2::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    return p;
}
