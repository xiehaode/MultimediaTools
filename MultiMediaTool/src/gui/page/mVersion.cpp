#include "mVersion.h"
#include "ui_mVersion.h"
#include "src/version.h"

mVersion::mVersion(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::version)
{
    ui->setupUi(this);

    ui->versionLabel->setText(QString("版本 %1").arg(VERSION_STRING));
    ui->branchLabel->setText(GIT_BRANCH);
    ui->commitLabel->setText(GIT_COMMIT_HASH);
    ui->buildTimeLabel->setText(BUILD_TIMESTAMP);
}

mVersion::~mVersion()
{
    delete ui;
}
