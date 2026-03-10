#include "grey.h"
#include "ui_grey.h"
#include <cstring>

grey::grey(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grey)
{
    ui->setupUi(this);
}

grey::~grey()
{
    delete ui;
}

param grey::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    return p;
}
