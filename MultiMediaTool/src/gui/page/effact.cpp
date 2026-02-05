#include "effact.h"
#include "ui_effact.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>

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
        qDebug() << "错误：转换器未初始化。请先点击'导出'选择路径。";
        return;
    }

    // 禁用按钮防止重复点击
    ui->ok->setEnabled(false);
    
    qDebug() << "开始处理视频...";
    int processRet = VideoTrans_Process(trans, effectType);
    
    if (processRet != 0) {
        qDebug() << "视频处理失败，错误码:" << processRet;
        // 如果处理失败，建议销毁重建以确保状态干净
        VideoTrans_Destroy(trans);
        trans = nullptr;
    } else {
        qDebug() << "视频处理完成！";
        // 处理成功后，重置底层状态但保留实例，方便下次直接使用
        VideoTrans_Reset(trans); 
    }

    ui->ok->setEnabled(true);
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

    // 使用 getSaveFileName 允许用户指定文件名和位置，避免目录拼接错误
    outFile = QFileDialog::getSaveFileName(this, "选择输出文件", QDir::currentPath(), "视频文件 (*.mp4)");
    
    if(outFile.isEmpty()){
        qDebug() << "未选择输出文件";
        return;
    }

    // 转换为本地路径并确保使用 UTF-8 编码传递给底层 C 接口
    // 许多底层库（如 FFmpeg）在 Windows 上期望 UTF-8 路径
    std::string utf8Input = QDir::toNativeSeparators(file).toUtf8().constData();
    std::string utf8Output = QDir::toNativeSeparators(outFile).toUtf8().constData();

    if (!trans) {
        trans = VideoTrans_Create();
    }

    if (!trans) {
        qDebug() << "无法创建视频转换器实例";
        return;
    }

    // 在独立线程中初始化，避免STA冲突
    QFuture<int> future = QtConcurrent::run([this, &utf8Input, &utf8Output]() -> int {
        return VideoTrans_Initialize(trans, utf8Input.c_str(), utf8Output.c_str());
    });
    
    // 等待初始化完成
    int initRet = future.result();
    
    // 如果因为分辨率非偶数导致失败 (-4)，尝试获取尺寸并规整（如果底层支持获取属性）
    // 注意：目前的底层逻辑在 Initialize 失败时会清理资源，所以这里我们直接提示用户
    if (initRet != 0) {
        qDebug() << "初始化失败，错误码:" << initRet;
        if (initRet == -4) {
            qDebug() << "提示：编码器要求视频宽度和高度必须是偶数。请检查输入视频分辨率。";
        }
        VideoTrans_Destroy(trans);
        trans = nullptr;
    } else {
        qDebug() << "初始化成功";
    }
}

