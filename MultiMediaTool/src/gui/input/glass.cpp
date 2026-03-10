#include "glass.h"
#include "ui_glass.h"

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
