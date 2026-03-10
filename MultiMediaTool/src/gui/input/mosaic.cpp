#include "mosaic.h"
#include "ui_mosaic.h"

mosaic::mosaic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mosaic)
{
    ui->setupUi(this);
}

mosaic::~mosaic()
{
    delete ui;
}
