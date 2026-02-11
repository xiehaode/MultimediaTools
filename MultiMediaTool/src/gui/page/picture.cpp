#include "picture.h"
#include "ui_picture.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QtConcurrent>
#include <QApplication>
#include <QCheckBox>
#include <QToolButton>
#include <QSpinBox>
#include <QFileInfo>
#include <QPixmap>

picture::picture(QWidget *parent) :
    QWidget(parent),  // 补充父类初始化
    ui(new Ui::picture),
    isProcessing(false)
{
    ui->setupUi(this);
    initUI();



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
    
    // 设置参数控件的默认值和范围
    ui->spinBox->setRange(1, 50);
    ui->spinBox->setValue(5);
    ui->spinBox->setSuffix(" px");
    ui->spinBox->setToolTip("参数一：影响效果的强度或范围");
    
    ui->spinBox_2->setRange(1, 50);
    ui->spinBox_2->setValue(3);
    ui->spinBox_2->setSuffix(" level");
    ui->spinBox_2->setToolTip("参数二：调整细节程度");
    
    // 设置按钮的提示信息
    ui->addFile->setToolTip("选择要处理的图片文件");
    ui->exportFile->setToolTip("选择处理后文件的保存位置");
    ui->ok->setToolTip("开始处理图片");
    ui->cancel->setToolTip("重置所有选项");
}

void picture::setupCheckBoxConnections()
{
    // 连接所有复选框的stateChanged信号（替代clicked）
    QList<QCheckBox*> checkBoxes = {
        ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
        ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
        ui->whitening2, ui->colorinvert
    };

    for (QCheckBox* box : checkBoxes) {
        // 关键：改用stateChanged信号，确保勾选状态变化能被正确捕获
        connect(box, &QCheckBox::stateChanged, this, &picture::onCheckBoxStateChanged);
    }

    // 连接按钮信号
    connect(ui->addFile, &QAbstractButton::clicked, this, &picture::on_addFile_clicked);
    connect(ui->exportFile, &QAbstractButton::clicked, this, &picture::on_exportFile_clicked);
    connect(ui->ok, &QAbstractButton::clicked, this, &picture::on_ok_clicked);

    connect(ui->cancel, &QAbstractButton::clicked, [this]() {
        // 取消逻辑：重置所有状态
        QList<QCheckBox*> checkBoxes = {
            ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
            ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
            ui->whitening2, ui->colorinvert
        };
        
        // ? 关键修复：阻止信号循环
        for (QCheckBox* box : checkBoxes) {
            box->blockSignals(true);
            box->setChecked(false);
            box->blockSignals(false);
        }
        
        effectType = noAction;
        file.clear();
        outFile.clear();
        
        // 重置按钮文本
        ui->addFile->setText("待处理文件");
        ui->exportFile->setText("输出位置");
        
        // 重置参数控件
        ui->spinBox->setValue(5);
        ui->spinBox_2->setValue(3);
    });
}

// 重构复选框状态变化处理函数（核心修复）
void picture::onCheckBoxStateChanged(int state)
{
    qDebug()<<"you check the CheckBox";
    QCheckBox* senderBox = qobject_cast<QCheckBox*>(sender());
    if (!senderBox) return;

    // 阻止信号循环处理
    if (senderBox->signalsBlocked()) {
        return;  // 如果信号被阻止，不处理
    }

    // 未勾选状态
    if (state == Qt::Unchecked) {
        effectType = noAction;
        return;
    }


    ensureSingleSelection(senderBox);

    // 设置对应的特效类型 + 打印调试信息（确认赋值生效）
    qDebug() << "勾选：" << senderBox->objectName();
    if (senderBox == ui->gray) {
        effectType = grayImage;
        qDebug() << "选中灰度效果";
    } else if (senderBox == ui->TextWatermark) {
        effectType = addTextWatermark;
        qDebug() << "选中文字水印效果";
    } else if (senderBox == ui->customOilPaintApprox) {
        effectType = customOilPaintApprox;

    } else if (senderBox == ui->OilPainting2) {
        effectType = applyOilPainting;

    } else if (senderBox == ui->Mosaic) {
        effectType = applyMosaic;

        senderBox->setFocus();
        
    } else if (senderBox == ui->FrostedGlass) {
        effectType = FrostedGlass;
        qDebug() << "选中磨砂玻璃效果";
    } else if (senderBox == ui->SkinSmoothing) {
        effectType = simpleSkinSmoothing;
        qDebug() << "选中磨皮效果";
    } else if (senderBox == ui->whitening) {
        effectType = Whitening;
        qDebug() << "选中美白1效果";
    } else if (senderBox == ui->whitening2) {
        effectType = Whitening2;
        qDebug() << "选中美白2效果";
    } else if (senderBox == ui->colorinvert) {
        effectType = invertImage;
        qDebug() << "选中颜色反转效果";
    }
    
    //  强制刷新UI，确保状态立即生效
    QApplication::processEvents();

    // 通用UI反馈：打印当前选中的特效类型
    qDebug() << "当前特效类型：" << effectType;
}

// 修复单选逻辑（确保其他复选框被取消）
void picture::ensureSingleSelection(QCheckBox* checkedBox)
{
    QList<QCheckBox*> checkBoxes = {
        ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
        ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
        ui->whitening2, ui->colorinvert
    };

    qDebug() << "ensureSingleSelection 开始执行，当前点击：" << checkedBox->objectName();
    
    for (QCheckBox* box : checkBoxes) {
        if (box != checkedBox) {
            bool wasChecked = box->isChecked();
            qDebug() << "检查复选框：" << box->objectName() << "，状态：" << wasChecked;
            
            if (wasChecked) {
                //  关键修复：阻止信号循环，避免互相干扰
                qDebug() << "正在取消：" << box->objectName();
                box->blockSignals(true);
                
                //  强制刷新UI，确保状态更新
                QApplication::processEvents();
                
                box->setChecked(false);
                
                //  清除可能的焦点问题
                if (box->hasFocus()) {
                    box->clearFocus();
                }
                
                box->blockSignals(false);
                qDebug() << "已取消：" << box->objectName();
            }
        }
    }
    
    qDebug() << "ensureSingleSelection 执行完成";
}

bool picture::isValidImageFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    // 检查文件是否存在
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }
    
    // 检查文件扩展名
    QString suffix = fileInfo.suffix().toLower();
    QStringList validExtensions = {"png", "jpg", "jpeg", "bmp", "gif", "tiff", "webp"};
    if (!validExtensions.contains(suffix)) {
        return false;
    }
    
    // 尝试加载图片验证完整性
    QPixmap pixmap(filePath);
    return !pixmap.isNull();
}

// 以下代码保持不变（on_ok_clicked/on_addFile_clicked/on_exportFile_clicked/showLoading/hideLoading）
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
    
    // 验证输入文件是否仍然有效
    if (!QFileInfo::exists(file)) {
        QMessageBox::warning(this, "错误", "选择的输入文件不存在或已被移动！");
        file.clear();
        ui->addFile->setText("待处理文件");
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

    // 获取参数值
    int p1 = ui->spinBox->value();
    int p2 = ui->spinBox_2->value();
    
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
        // 验证文件是否为有效图片
        if (!isValidImageFile(selectedFile)) {
            QMessageBox::warning(this, "警告", "选择的文件不是有效的图片文件或文件已损坏！");
            return;
        }
        
        file = selectedFile;
        qDebug() << "Selected file:" << file;
        
        // 更新按钮文本显示文件名
        QFileInfo fileInfo(file);
        QString shortFileName = fileInfo.fileName();
        if (shortFileName.length() > 20) {
            shortFileName = shortFileName.left(17) + "...";
        }
        ui->addFile->setText("已选择:\n" + shortFileName);
        
        // 如果没有选择输出路径，自动生成一个
        if (outFile.isEmpty()) {
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
        
        // 更新按钮文本显示输出文件名
        QFileInfo fileInfo(outFile);
        QString shortFileName = fileInfo.fileName();
        if (shortFileName.length() > 20) {
            shortFileName = shortFileName.left(17) + "...";
        }
        ui->exportFile->setText("输出到:\n" + shortFileName);
    }
}

void picture::showLoading()
{

    // 禁用父窗口
    setEnabled(false);
}

void picture::hideLoading()
{

    // 启用父窗口
    setEnabled(true);
}
