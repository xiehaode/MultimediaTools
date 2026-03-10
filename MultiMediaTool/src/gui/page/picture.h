#ifndef PICTURE_H
#define PICTURE_H
#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <QLabel>
#include <QMovie>
#include <QTimer>
#include <QSpinBox>
#include "../../OpenCVTools/OpenCVFFMpegTools.h"

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


namespace Ui {
class picture;
}

class picture : public QWidget
{
    Q_OBJECT

public:
    explicit picture(QWidget *parent = nullptr);
    ~picture();
    void initUI();

private slots:
    // 改用stateChanged信号（更稳定）
    void onCheckBoxStateChanged(int state);
    void on_ok_clicked();
    void on_addFile_clicked();
    void on_exportFile_clicked();
    void showLoading();
    void hideLoading();

    void on_pushButton_clicked();
    void updataParamUi();

private:
    Ui::picture *ui;
    QString file;
    QString outFile;
    void *translator = nullptr;
    func effectType = noAction;  // 确保类型统一
    bool isProcessing;


    void setupCheckBoxConnections();
    void ensureSingleSelection(QCheckBox* checkedBox);
    bool isValidImageFile(const QString& filePath);
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

#endif // PICTURE_H
