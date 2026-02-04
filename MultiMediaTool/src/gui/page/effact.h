#ifndef EFFACT_H
#define EFFACT_H
#include "src/base/pagebase.h"
#include "OpenCVFFMpegTools.h"
#include <QWidget>
#include <QCheckBox>
#include <QFile>

namespace Ui {
class effact;
}

class effact : public QWidget
{
    Q_OBJECT

public:
    explicit effact(QWidget *parent = nullptr);
    ~effact();
    void initUI();

private slots:
    void onCheckBoxClicked();

    void on_ok_clicked();


    void on_addFile_clicked();

    void on_exportFile_clicked();

private:
    Ui::effact *ui;
    QString file;
    QString outFile;
    void *trans=NULL;
    func effectType=noAction;
    void setupCheckBoxConnections();
    void ensureSingleSelection(QCheckBox* checkedBox);
};

#endif // EFFACT_H
