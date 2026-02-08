#ifndef EFFACT_H
#define EFFACT_H
#include "src/base/pagebase.h"
#include "OpenCVFFMpegTools.h"
#include "formatChange.h"
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

    // 格式转换相关槽函数
    void on_btnImport_clicked();
    void on_comboBoxSrc_currentIndexChanged(int index);
    void on_pushButton_clicked();

private:
    Ui::effact *ui;
    QString file;
    QString outFile;
    void *trans=NULL;
    void *m_processor = nullptr;
    QString m_importPath;
    func effectType=noAction;
    void setupCheckBoxConnections();
    void ensureSingleSelection(QCheckBox* checkedBox);
};

#endif // EFFACT_H
