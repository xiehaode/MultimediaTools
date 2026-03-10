#include "oilpainting.h"
#include "ui_oilpainting.h"
#include <cstring>

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

param oilPainting::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));
    p.iparam1 = ui->spinBox_detail->value();  // 油画细节
    p.dparam1 = ui->doubleSpinBox_intensity->value();  // 油画强度
    return p;
}
