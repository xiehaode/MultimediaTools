#include "effact.h"
#include "ui_effact.h"

effact::effact(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::effact)
{
    ui->setupUi(this);
}

effact::~effact()
{
    delete ui;
}
