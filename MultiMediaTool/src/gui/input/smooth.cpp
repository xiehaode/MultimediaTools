#include "smooth.h"
#include "ui_smooth.h"

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
