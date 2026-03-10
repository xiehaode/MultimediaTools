#include "reverse.h"
#include "ui_reverse.h"

reverse::reverse(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::reverse)
{
    ui->setupUi(this);
}

reverse::~reverse()
{
    delete ui;
}
