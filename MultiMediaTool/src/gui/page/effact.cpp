#include "effact.h"
#include "ui_effact.h"
#include <QFileDialog>


effact::effact(QWidget *parent) :
    ui(new Ui::effact)
{
    ui->setupUi(this);
    initUI();
    trans = VideoTrans_Create();
    if (!trans) {
        qDebug()<<"VideoTrans_Create fair";
    }
}

effact::~effact()
{
    if (trans) {
        VideoTrans_Destroy(trans);
        trans = nullptr;
    }
    delete ui;
}

void effact::initUI()
{
    setupCheckBoxConnections();
}

void effact::setupCheckBoxConnections()
{
    // 连接所有复选框的clicked信号到同一个槽函数
    connect(ui->gray, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->TextWatermark, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->customOilPaintApprox, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->OilPainting2, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->Mosaic, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->FrostedGlass, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->SkinSmoothing, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->whitening, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->whitening2, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
    connect(ui->colorinvert, &QCheckBox::clicked, this, &effact::onCheckBoxClicked);
}

void effact::onCheckBoxClicked()
{
    QCheckBox* clickedBox = qobject_cast<QCheckBox*>(sender());
    if (clickedBox) {
        ensureSingleSelection(clickedBox);
        
        // 根据选中的复选框确定对应的func枚举值

        if (clickedBox == ui->gray) {
            effectType = grayImage;
        } else if (clickedBox == ui->TextWatermark) {
            effectType = addTextWatermark;
        } else if (clickedBox == ui->customOilPaintApprox) {
            effectType = customOilPaintApprox;
        } else if (clickedBox == ui->OilPainting2) {
            effectType = applyOilPainting;
        } else if (clickedBox == ui->Mosaic) {
            effectType = applyMosaic;
        } else if (clickedBox == ui->FrostedGlass) {
            effectType = FrostedGlass;
        } else if (clickedBox == ui->SkinSmoothing) {
            effectType = simpleSkinSmoothing;
        } else if (clickedBox == ui->whitening) {
            effectType = Whitening;
        } else if (clickedBox == ui->whitening2) {
            effectType = Whitening2;
        } else if (clickedBox == ui->colorinvert) {
            effectType = invertImage;
        }
        

    }
}

void effact::ensureSingleSelection(QCheckBox* checkedBox)
{
    if (checkedBox->isChecked()) {
        // 如果当前复选框被选中，取消其他所有复选框的选中状态
        QList<QCheckBox*> checkBoxes = {
            ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
            ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
            ui->whitening2, ui->colorinvert
        };
        
        for (QCheckBox* box : checkBoxes) {
            if (box != checkedBox) {
                box->setChecked(false);
            }
        }
    }
}

void effact::on_ok_clicked()
{
    if (!trans) {
        qDebug() << "视频转换器未初始化或已销毁";
        return;
    }

    int processRet = VideoTrans_Process(trans, effectType);
    if (processRet != 0) {
        qDebug()<<"视频处理失败，错误码:"<<processRet;
    }
    
    VideoTrans_Destroy(trans);
    trans = nullptr;

    printf("视频处理完成！输出路径：%s\n", outFile.toStdString().c_str());
}


void effact::on_addFile_clicked()
{
     file =QFileDialog::getOpenFileName();
     if(file.isEmpty()){
         qDebug()<<"file is empty";
     }
}


void effact::on_exportFile_clicked()
{
    if (file.isEmpty()) {
        qDebug() << "未选择输入文件";
        return;
    }

    outFile =QFileDialog::getExistingDirectory()+"/output.mp4";
    if(outFile.isEmpty()){
        qDebug()<<"输出目录为空";
        return;
    }

    if (!trans) {
        trans = VideoTrans_Create();
    }

    if (!trans) {
        qDebug() << "无法创建视频转换器";
        return;
    }

    int initRet = VideoTrans_Initialize(trans, file.toStdString().c_str(), outFile.toStdString().c_str());
    if (initRet != 0) {
        qDebug()<<"初始化失败，错误码:"<<initRet;
        VideoTrans_Destroy(trans);
        trans = nullptr;
    }
}

