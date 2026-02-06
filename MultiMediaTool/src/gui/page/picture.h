#ifndef PICTURE_H
#define PICTURE_H
#include <QWidget>
#include <QString>
#include <QCheckBox>
#include <QLabel>
#include <QMovie>
#include <QTimer>
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
    void onCheckBoxClicked();
    void on_ok_clicked();
    void on_addFile_clicked();

    void showLoading();
    void hideLoading();

private:
    Ui::picture *ui;
    QString file;
    QString outFile;
    void *translator = nullptr;
    func effectType = noAction;
    bool isProcessing;
    
    // 加载动画
    QLabel *loadingLabel;
    QMovie *loadingMovie;
    
    void setupCheckBoxConnections();
    void ensureSingleSelection(QCheckBox* checkedBox);
};

#endif // PICTURE_H
