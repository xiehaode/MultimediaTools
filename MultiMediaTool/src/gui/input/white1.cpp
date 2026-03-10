#include "white1.h"
#include "ui_white1.h"

white1::white1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::white1)
{
    ui->setupUi(this);
}

white1::~white1()
{
    delete ui;
}
