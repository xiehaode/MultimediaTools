#include "conacat.h"
#include "ui_conacat.h"

conacat::conacat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::conacat)
{
    ui->setupUi(this);
}

conacat::~conacat()
{
    delete ui;
}
