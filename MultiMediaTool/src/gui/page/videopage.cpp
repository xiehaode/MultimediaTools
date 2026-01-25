#include "videopage.h"
#include "ui_videopage.h"

videoPage::videoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::videoPage)
{
    ui->setupUi(this);
}

videoPage::~videoPage()
{
    delete ui;
}
