#include "oilpainting.h"
#include "ui_oilpainting.h"

oilPainting::oilPainting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::oilPainting)
{
    ui->setupUi(this);
}

oilPainting::~oilPainting()
{
    delete ui;
}
