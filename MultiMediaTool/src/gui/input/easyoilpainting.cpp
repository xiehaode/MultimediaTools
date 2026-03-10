#include "easyoilpainting.h"
#include "ui_easyoilpainting.h"
#include <cstring>

easyOilPainting::easyOilPainting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::easyOilPainting)
{
    ui->setupUi(this);
}

easyOilPainting::~easyOilPainting()
{
    delete ui;
}

param easyOilPainting::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));

    p.iparam1 = ui->spinBox_radius->value();
    p.dparam1 = ui->doubleSpinBox_level->value();

    return p;
}
