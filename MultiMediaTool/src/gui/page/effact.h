#ifndef EFFACT_H
#define EFFACT_H
#include "src/base/pagebase.h"
#include "OpenCVFFMpegTools.h"
#include "src/base/dragdrophandler.h"
#include "formatChange.h"
#include <QWidget>
#include <QCheckBox>
#include <QFile>

// Input控件头文件
#include "src/gui/input/glass.h"
#include "src/gui/input/grey.h"
#include "src/gui/input/mosaic.h"
#include "src/gui/input/oilpainting.h"
#include "src/gui/input/reverse.h"
#include "src/gui/input/smooth.h"
#include "src/gui/input/white1.h"
#include "src/gui/input/white2.h"
#include "src/gui/input/watertext.h"
#include "src/gui/input/easyoilpainting.h"

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
    void setupInputWidgets();
    void updateParameterWidget();
private:
    // Input控件实例
    glass *m_glassWidget = nullptr;
    grey *m_greyWidget = nullptr;
    mosaic *m_mosaicWidget = nullptr;
    oilPainting *m_oilPaintingWidget = nullptr;
    reverse *m_reverseWidget = nullptr;
    smooth *m_smoothWidget = nullptr;
    white1 *m_white1Widget = nullptr;
    white2 *m_white2Widget = nullptr;
    waterText *m_waterTextWidget = nullptr;
    easyOilPainting *m_easyOilPaintingWidget = nullptr;
};

#endif // EFFACT_H
