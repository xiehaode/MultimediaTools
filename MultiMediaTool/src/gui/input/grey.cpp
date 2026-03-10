#include "grey.h"
#include "ui_grey.h"

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
