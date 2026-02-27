#ifndef EFFACT_H
#define EFFACT_H
#include "src/base/pagebase.h"
#include "OpenCVFFMpegTools.h"
#include "src/base/dragdrophandler.h"
#include "formatChange.h"
#include <QWidget>
#include <QCheckBox>
#include <QFile>

enum step{
    setNone = 1,
    setFile1 =2,
    setFile2 =3,
    finished
};

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
    void updataParamUi();
private:
    Ui::effact *ui;
    step s = setNone;
    QString file;
    QString outFile;
    void *trans=NULL;
    void *m_processor = nullptr;
    QString m_importPath;
    func effectType=noAction;
    DragDropHandler *m_dragHandler;
private:
    void setupCheckBoxConnections();
    void ensureSingleSelection(QCheckBox* checkedBox);
    void handleDroppedFiles(const QList<QString> &filePaths);
};

#endif // EFFACT_H
