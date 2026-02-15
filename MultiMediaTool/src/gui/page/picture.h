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
};

#endif // PICTURE_H
