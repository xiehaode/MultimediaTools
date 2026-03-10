#include "glass.h"
#include "ui_glass.h"
#include <cstring>

glass::glass(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::glass)
{
    ui->setupUi(this);
}

glass::~glass()
{
    delete ui;
}

param glass::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    return p;
}
