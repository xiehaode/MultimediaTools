#include "version.h"
#include "ui_version.h"

version::version(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::version)
{
    ui->setupUi(this);
}

version::~version()
{
    delete ui;
}
