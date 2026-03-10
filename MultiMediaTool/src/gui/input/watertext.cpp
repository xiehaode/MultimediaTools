#include "watertext.h"
#include "ui_watertext.h"
#include <cstring>

waterText::waterText(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::waterText)
{
    ui->setupUi(this);
}

waterText::~waterText()
{
    delete ui;
}

param waterText::getParams() const
{
    param p;
    memset(&p, 0, sizeof(p));

    p.iparam1 = ui->spinBox_x->value();
    p.iparam2 = ui->spinBox_y->value();
    p.dparam1 = ui->doubleSpinBox_scale->value();
    p.iparam3 = ui->spinBox_thickness->value();
    p.iparam4 = ui->spinBox_r->value();
    p.iparam5 = ui->spinBox_g->value();

    std::string text = ui->lineEdit_watermarkText->text().toStdString();
    strncpy(p.arr, text.c_str(), sizeof(p.arr) - 1);
    p.arr[sizeof(p.arr) - 1] = '\0';

    return p;
}
