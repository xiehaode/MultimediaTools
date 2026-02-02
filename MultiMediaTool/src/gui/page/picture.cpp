#include "picture.h"
#include "ui_picture.h"

picture::picture(QWidget *parent) :
    ui(new Ui::picture)
{
    ui->setupUi(this);
}

picture::~picture()
{
    delete ui;
}
