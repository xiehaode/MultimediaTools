#include "picture.h"
#include "ui_picture.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QtConcurrent>

picture::picture(QWidget *parent) :
    ui(new Ui::picture), isProcessing(false)
{
    ui->setupUi(this);
    initUI();
    
    // 初始化加载动画
    loadingLabel = new QLabel(this);
    loadingMovie = new QMovie(":/res/loading.gif");
    loadingLabel->setMovie(loadingMovie);
    loadingLabel->setWindowFlags(Qt::FramelessWindowHint);
    loadingLabel->setAttribute(Qt::WA_TranslucentBackground);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setFixedSize(200, 200);
    
    // 设置加载动画样式
    loadingLabel->setStyleSheet("QLabel { background-color: rgba(0, 0, 0, 150); border-radius: 10px; }");
    
    translator = CvTranslator_Create();
    if (!translator) {
        qDebug() << "CvTranslator_Create failed";
    }
}

picture::~picture()
{
    if (translator) {
        CvTranslator_Destroy(translator);
    }
    delete ui;
}

void picture::initUI()
{
    setupCheckBoxConnections();
}

void picture::setupCheckBoxConnections()
{
    // 连接所有复选框的点击信号
    QList<QCheckBox*> checkBoxes = {
        ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
        ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
        ui->whitening2, ui->colorinvert
    };
    
    for (QCheckBox* box : checkBoxes) {
        connect(box, &QCheckBox::clicked, this, &picture::onCheckBoxClicked);
    }
    
    // 连接按钮信号
    connect(ui->addFile, &QPushButton::clicked, this, &picture::on_addFile_clicked);

    connect(ui->ok, &QPushButton::clicked, this, &picture::on_ok_clicked);
}

void picture::onCheckBoxClicked()
{
    QCheckBox* senderBox = qobject_cast<QCheckBox*>(sender());
    if (!senderBox) return;
    
    ensureSingleSelection(senderBox);
    
    // 设置对应的特效类型
    if (senderBox == ui->gray) {
        effectType = grayImage;
    } else if (senderBox == ui->TextWatermark) {
        effectType = addTextWatermark;
    } else if (senderBox == ui->customOilPaintApprox) {
        effectType = customOilPaintApprox;
    } else if (senderBox == ui->OilPainting2) {
        effectType = applyOilPainting;
    } else if (senderBox == ui->Mosaic) {
        effectType = applyMosaic;
    } else if (senderBox == ui->FrostedGlass) {
        effectType = FrostedGlass;
    } else if (senderBox == ui->SkinSmoothing) {
        effectType = simpleSkinSmoothing;
    } else if (senderBox == ui->whitening) {
        effectType = Whitening;
    } else if (senderBox == ui->whitening2) {
        effectType = Whitening2;
    } else if (senderBox == ui->colorinvert) {
        effectType = invertImage;
    }
}

void picture::ensureSingleSelection(QCheckBox* checkedBox)
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

void picture::on_ok_clicked()
{
    // 检查是否正在处理
    if (isProcessing) {
        QMessageBox::information(this, "提示", "正在处理中，请稍候...");
        return;
    }

    // 检查是否已经选择了效果
    if (effectType == noAction) {
        QMessageBox::information(this, "提示", "请先选择一个图片效果");
        return;
    }

    // 检查是否已经选择了输入文件
    if (file.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择输入文件");
        return;
    }

    // 使用 getSaveFileName 允许用户指定文件名和位置
    outFile = QFileDialog::getSaveFileName(this, "选择输出文件", QDir::currentPath(), "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    
    if (outFile.isEmpty()) {
        qDebug() << "未选择输出文件";
        return;
    }

    // 显示加载动画
    showLoading();
    
    // 设置处理标志
    isProcessing = true;
    
    // 禁用按钮
    ui->addFile->setEnabled(false);
    ui->exportFile->setEnabled(false);
    ui->ok->setEnabled(false);

    if (!translator) {
        translator = CvTranslator_Create();
    }

    if (!translator) {
        hideLoading();
        isProcessing = false;
        ui->addFile->setEnabled(true);
        ui->exportFile->setEnabled(true);
        ui->ok->setEnabled(true);
        qDebug() << "无法创建图片转换器实例";
        QMessageBox::critical(this, "错误", "图片转换器创建失败");
        return;
    }

    // 转换为本地路径并确保使用 UTF-8 编码传递给底层 C 接口
    std::string utf8Input = QDir::toNativeSeparators(file).toUtf8().constData();
    std::string utf8Output = QDir::toNativeSeparators(outFile).toUtf8().constData();

    // 在独立线程中处理图片
    QFuture<bool> future = QtConcurrent::run([this, &utf8Input, &utf8Output]() -> bool {
        switch (effectType) {
            case grayImage:
                return CvTranslator_GrayImage_File(translator, utf8Input.c_str(), utf8Output.c_str());
            case addTextWatermark:
                return CvTranslator_AddTextWatermark_File(translator, utf8Input.c_str(), utf8Output.c_str(), "Watermark");
            case customOilPaintApprox:
            case applyOilPainting:
                return CvTranslator_OilPainting_File(translator, utf8Input.c_str(), utf8Output.c_str(), 5, 1.0);
            case applyMosaic:
                return CvTranslator_Mosaic_File(translator, utf8Input.c_str(), utf8Output.c_str(), 0, 0, 0, 0, 10);
            case FrostedGlass:
                return CvTranslator_FrostedGlass_File(translator, utf8Input.c_str(), utf8Output.c_str());
            case simpleSkinSmoothing:
                return CvTranslator_SkinSmoothing_File(translator, utf8Input.c_str(), utf8Output.c_str());
            case Whitening:
                return CvTranslator_Whitening_File(translator, utf8Input.c_str(), utf8Output.c_str());
            case Whitening2:
                return CvTranslator_Whitening2_File(translator, utf8Input.c_str(), utf8Output.c_str());
            case invertImage:
                return CvTranslator_Invert_File(translator, utf8Input.c_str(), utf8Output.c_str());
            default:
                return false;
        }
    });
    
    // 等待处理完成
    bool success = future.result();
    
    hideLoading();
    isProcessing = false;
    ui->addFile->setEnabled(true);
    ui->exportFile->setEnabled(true);
    ui->ok->setEnabled(true);
    
    if (success) {
        QMessageBox::information(this, "成功", "图片处理完成！");
    } else {
        QMessageBox::critical(this, "错误", "图片处理失败");
    }
}

void picture::on_addFile_clicked()
{
    file = QFileDialog::getOpenFileName();
    if (file.isEmpty()) {
        qDebug() << "file is empty";
    }
}



void picture::showLoading()
{
    // 设置加载动画在窗口中心
    loadingLabel->move((this->width() - 200) / 2, (this->height() - 200) / 2);
    loadingLabel->show();
    loadingMovie->start();
    
    // 禁用父窗口
    setEnabled(false);
}

void picture::hideLoading()
{
    loadingMovie->stop();
    loadingLabel->hide();
    
    // 启用父窗口
    setEnabled(true);
}
