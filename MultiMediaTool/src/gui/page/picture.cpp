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
    ui->paramGroupBox->hide();
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
    
    // 连接按钮信号 - 使用 QAbstractButton::clicked 确保兼容 QToolButton 和 QPushButton
    connect(ui->addFile, &QAbstractButton::clicked, this, &picture::on_addFile_clicked);
    connect(ui->exportFile, &QAbstractButton::clicked, this, &picture::on_exportFile_clicked);
    connect(ui->ok, &QAbstractButton::clicked, this, &picture::on_ok_clicked);

    connect(ui->cancel, &QAbstractButton::clicked, [this]() {
        // 取消逻辑：重置所有状态
        for (QCheckBox* box : {
            ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
            ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
            ui->whitening2, ui->colorinvert
        }) {
            box->setChecked(false);
        }
        effectType = noAction;
        ui->paramGroupBox->hide();
        file.clear();
    });
}

void picture::onCheckBoxClicked()
{
    QCheckBox* senderBox = qobject_cast<QCheckBox*>(sender());
    if (!senderBox) return;
    
    // 如果是取消勾选
    if (!senderBox->isChecked()) {
        effectType = noAction;
        ui->paramGroupBox->hide();
        return;
    }
    
    ensureSingleSelection(senderBox);
    
    // 隐藏参数面板，除非选中的是需要参数的特效
    ui->paramGroupBox->hide();
    
    // 设置对应的特效类型
    if (senderBox == ui->gray) {
        effectType = grayImage;
    } else if (senderBox == ui->TextWatermark) {
        effectType = addTextWatermark;
    } else if (senderBox == ui->customOilPaintApprox) {
        effectType = customOilPaintApprox;
        ui->paramGroupBox->show();
        ui->param1Label->setText("半径:");
        ui->param2Label->setText("平滑度:");
        ui->param1SpinBox->setValue(5);
        ui->param2SpinBox->setValue(1.0);
        ui->param2Label->show();
        ui->param2SpinBox->show();
    } else if (senderBox == ui->OilPainting2) {
        effectType = applyOilPainting;
        ui->paramGroupBox->show();
        ui->param1Label->setText("半径:");
        ui->param2Label->setText("平滑度:");
        ui->param1SpinBox->setValue(5);
        ui->param2SpinBox->setValue(1.0);
        ui->param2Label->show();
        ui->param2SpinBox->show();
    } else if (senderBox == ui->Mosaic) {
        effectType = applyMosaic;
        ui->paramGroupBox->show();
        ui->param1Label->setText("块大小:");
        ui->param1SpinBox->setValue(10);
        ui->param2Label->hide();
        ui->param2SpinBox->hide();
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

    // 如果没有预选输出路径，则弹出对话框
    if (outFile.isEmpty()) {
        outFile = QFileDialog::getSaveFileName(this, "选择输出文件", QDir::currentPath(), "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    }
    
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
    // ... 后面代码保持不变 ...


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
    std::string utf8InputStr = QDir::toNativeSeparators(file).toUtf8().toStdString();
    std::string utf8OutputStr = QDir::toNativeSeparators(outFile).toUtf8().toStdString();

    // 在独立线程中处理图片
    int p1 = ui->param1SpinBox->value();
    double p2 = ui->param2SpinBox->value();
    
    // 注意：这里必须按值捕获字符串，因为它们是局部变量
    QFuture<bool> future = QtConcurrent::run([this, utf8InputStr, utf8OutputStr, p1, p2]() -> bool {
        const char* inputPath = utf8InputStr.c_str();
        const char* outputPath = utf8OutputStr.c_str();
        
        switch (effectType) {
            case grayImage:
                return CvTranslator_GrayImage_File(translator, inputPath, outputPath);
            case addTextWatermark:
                return CvTranslator_AddTextWatermark_File(translator, inputPath, outputPath, "Watermark");
            case customOilPaintApprox:
            case applyOilPainting:
                return CvTranslator_OilPainting_File(translator, inputPath, outputPath, p1, p2);
            case applyMosaic:
                return CvTranslator_Mosaic_File(translator, inputPath, outputPath, 0, 0, 0, 0, p1);
            case FrostedGlass:
                return CvTranslator_FrostedGlass_File(translator, inputPath, outputPath);
            case simpleSkinSmoothing:
                return CvTranslator_SkinSmoothing_File(translator, inputPath, outputPath);
            case Whitening:
                return CvTranslator_Whitening_File(translator, inputPath, outputPath);
            case Whitening2:
                return CvTranslator_Whitening2_File(translator, inputPath, outputPath);
            case invertImage:
                return CvTranslator_Invert_File(translator, inputPath, outputPath);
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
    QString selectedFile = QFileDialog::getOpenFileName(this, "选择图片文件", QDir::currentPath(), "图片文件 (*.png *.jpg *.jpeg *.bmp);;所有文件 (*.*)");
    if (!selectedFile.isEmpty()) {
        file = selectedFile;
        qDebug() << "Selected file:" << file;
        // 如果没有选择输出路径，自动生成一个
        if (outFile.isEmpty()) {
            QFileInfo fileInfo(file);
            outFile = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_processed." + fileInfo.suffix();
        }
    } else {
        qDebug() << "file selection cancelled or empty";
    }
}

void picture::on_exportFile_clicked()
{
    QString selectedOutFile = QFileDialog::getSaveFileName(this, "选择输出文件", outFile.isEmpty() ? QDir::currentPath() : outFile, "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    if (!selectedOutFile.isEmpty()) {
        outFile = selectedOutFile;
        qDebug() << "Output file set to:" << outFile;
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
