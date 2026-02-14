#include "picture.h"
#include "ui_picture.h"
#include "src/utils/lan_util.h"
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
    QWidget(parent),  // 父类初始化
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

    // 设置微调框控件的默认值及范围
    ui->spinBox->setRange(1, 50);
    ui->spinBox->setValue(5);
    ui->spinBox->setSuffix(gbk_to_utf8(" px").c_str());
    ui->spinBox->setToolTip(gbk_to_utf8("设置阴影效果的强度或范围").c_str());

    ui->spinBox_2->setRange(1, 50);
    ui->spinBox_2->setValue(3);
    ui->spinBox_2->setSuffix(gbk_to_utf8(" level").c_str());
    ui->spinBox_2->setToolTip(gbk_to_utf8("设置油画细节程度").c_str());

    // 设置按钮提示显示信息
    ui->addFile->setToolTip(gbk_to_utf8("选择要处理的图片文件").c_str());
    ui->exportFile->setToolTip(gbk_to_utf8("选择保存文件的目标位置").c_str());
    ui->ok->setToolTip(gbk_to_utf8("开始处理图片").c_str());
    ui->cancel->setToolTip(gbk_to_utf8("取消当前选择").c_str());
}

void picture::setupCheckBoxConnections()
{
    // 给复选框绑定stateChanged信号，代替clicked
    QList<QCheckBox*> checkBoxes = {
        ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
        ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
        ui->whitening2, ui->colorinvert
    };

    for (QCheckBox* box : checkBoxes) {
        // 控件绑定stateChanged信号，确保选中状态变化能被正确触发
        connect(box, &QCheckBox::stateChanged, this, &picture::onCheckBoxStateChanged);
    }

    // 绑定按钮信号
    connect(ui->addFile, &QAbstractButton::clicked, this, &picture::on_addFile_clicked);
    connect(ui->exportFile, &QAbstractButton::clicked, this, &picture::on_exportFile_clicked);
    connect(ui->ok, &QAbstractButton::clicked, this, &picture::on_ok_clicked);

    connect(ui->cancel, &QAbstractButton::clicked, [this]() {
        // 取消处理并重置界面状态
        QList<QCheckBox*> checkBoxes = {
            ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
            ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
            ui->whitening2, ui->colorinvert
        };

        // 控件修改时关闭信号循环
        for (QCheckBox* box : checkBoxes) {
            box->blockSignals(true);
            box->setChecked(false);
            box->blockSignals(false);
        }

        effectType = noAction;
        file.clear();
        outFile.clear();

        // 设置按钮文字
        ui->addFile->setText(gbk_to_utf8("添加文件").c_str());
        ui->exportFile->setText(gbk_to_utf8("导出位置").c_str());

        // 设置微调框
        ui->spinBox->setValue(5);
        ui->spinBox_2->setValue(3);
    });
}

// 响应复选框选中状态变化，更新对应的处理类型
void picture::onCheckBoxStateChanged(int state)
{
    qDebug()<<"you check the CheckBox";
    QCheckBox* senderBox = qobject_cast<QCheckBox*>(sender());
    if (!senderBox) return;

    // 防止信号循环调用
    if (senderBox->signalsBlocked()) {
        return;
    }

    // 未选中状态
    if (state == Qt::Unchecked) {
        effectType = noAction;
        return;
    }

    ensureSingleSelection(senderBox);

    // 设置对应的特效类型 + 打印调试信息，确认具体的特效
    qDebug() << gbk_to_utf8("已选择").c_str() << senderBox->objectName();
    if (senderBox == ui->gray) {
        effectType = grayImage;
        qDebug() << gbk_to_utf8("选择灰度效果").c_str();
    } else if (senderBox == ui->TextWatermark) {
        effectType = addTextWatermark;
        qDebug() << gbk_to_utf8("选择文字水印效果").c_str();
    } else if (senderBox == ui->customOilPaintApprox) {
        effectType = customOilPaintApprox;

    } else if (senderBox == ui->OilPainting2) {
        effectType = applyOilPainting;

    } else if (senderBox == ui->Mosaic) {
        effectType = applyMosaic;

        senderBox->setFocus();

    } else if (senderBox == ui->FrostedGlass) {
        effectType = FrostedGlass;
        qDebug() << gbk_to_utf8("选择毛玻璃效果").c_str();
    } else if (senderBox == ui->SkinSmoothing) {
        effectType = simpleSkinSmoothing;
        qDebug() << gbk_to_utf8("选择磨皮效果").c_str();
    } else if (senderBox == ui->whitening) {
        effectType = Whitening;
        qDebug() << gbk_to_utf8("选择美白1效果").c_str();
    } else if (senderBox == ui->whitening2) {
        effectType = Whitening2;
        qDebug() << gbk_to_utf8("选择美白2效果").c_str();
    } else if (senderBox == ui->colorinvert) {
        effectType = invertImage;
        qDebug() << gbk_to_utf8("选择颜色反转效果").c_str();
    }


    QApplication::processEvents();


    qDebug() << gbk_to_utf8("当前效果类型：").c_str() << effectType;
}

void picture::ensureSingleSelection(QCheckBox* checkedBox)
{
    QList<QCheckBox*> checkBoxes = {
        ui->gray, ui->TextWatermark, ui->customOilPaintApprox, ui->OilPainting2,
        ui->Mosaic, ui->FrostedGlass, ui->SkinSmoothing, ui->whitening,
        ui->whitening2, ui->colorinvert
    };

    qDebug() << gbk_to_utf8("ensureSingleSelection 开始执行，当前选中").c_str() << checkedBox->objectName();

    for (QCheckBox* box : checkBoxes) {
        if (box != checkedBox) {
            bool wasChecked = box->isChecked();
            qDebug() << gbk_to_utf8("复选框").c_str() << box->objectName() << gbk_to_utf8("状态：").c_str() << wasChecked;

            if (wasChecked) {
                // 控件修改时关闭信号循环，避免递归调用
                qDebug() << gbk_to_utf8("取消选中").c_str() << box->objectName();
                box->blockSignals(true);

                // 强制刷新UI，确保状态更新
                QApplication::processEvents();

                box->setChecked(false);

                // 清除可能的焦点状态
                if (box->hasFocus()) {
                    box->clearFocus();
                }

                box->blockSignals(false);
                qDebug() << gbk_to_utf8("取消信号").c_str() << box->objectName();
            }
        }
    }

    qDebug() << gbk_to_utf8("ensureSingleSelection 执行完毕").c_str();
}

bool picture::isValidImageFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);

    // 检查文件是否存在
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    // 检查文件后缀名
    QString suffix = fileInfo.suffix().toLower();
    QStringList validExtensions = {"png", "jpg", "jpeg", "bmp", "gif", "tiff", "webp"};
    if (!validExtensions.contains(suffix)) {
        return false;
    }

    // 加载图片验证文件有效性
    QPixmap pixmap(filePath);
    return !pixmap.isNull();
}

// 补充异常捕获代码（on_ok_clicked/on_addFile_clicked/on_exportFile_clicked/showLoading/hideLoading）
void picture::on_ok_clicked()
{
    // 检查是否正在处理
    if (isProcessing) {
        QMessageBox::information(this, gbk_to_utf8("提示").c_str(), gbk_to_utf8("正在处理中，请稍候...").c_str());
        return;
    }

    // 检查是否已经选择了特效
    if (effectType == noAction) {
        QMessageBox::information(this, gbk_to_utf8("提示").c_str(), gbk_to_utf8("请先选择一个图片效果").c_str());
        return;
    }

    // 检查是否已经选择了输入文件
    if (file.isEmpty()) {
        QMessageBox::information(this, gbk_to_utf8("提示").c_str(), gbk_to_utf8("请先选择输入文件").c_str());
        return;
    }

    // 验证输入文件是否依然有效
    if (!QFileInfo::exists(file)) {
        QMessageBox::warning(this, gbk_to_utf8("警告").c_str(), gbk_to_utf8("选择的文件不存在或已被移动").c_str());
        file.clear();
        ui->addFile->setText(gbk_to_utf8("添加文件").c_str());
        return;
    }

    // 如果用户未预设输出路径，则弹出对话框
    if (outFile.isEmpty()) {
        outFile = QFileDialog::getSaveFileName(this, gbk_to_utf8("选择输出文件").c_str(), QDir::currentPath(), gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp)").c_str());
    }

    if (outFile.isEmpty()) {
        qDebug() << gbk_to_utf8("未选择输出文件").c_str();
        return;
    }

    // 显示加载状态
    showLoading();

    // 设置处理标识
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
        qDebug() << gbk_to_utf8("无法创建图片转换引擎实例").c_str();
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("图片转换引擎实例失败").c_str());
        return;
    }

    // 转换为本地路径，确保使用 UTF-8 编码传递给底层 C 接口
    std::string utf8InputStr = QDir::toNativeSeparators(file).toUtf8().toStdString();
    std::string utf8OutputStr = QDir::toNativeSeparators(outFile).toUtf8().toStdString();

    // 获取参数值
    int p1 = ui->spinBox->value();
    int p2 = ui->spinBox_2->value();

    // 注意：此处硬编码参数值，实际项目中建议做成可配置项
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
        QMessageBox::information(this, gbk_to_utf8("成功").c_str(), gbk_to_utf8("图片处理完成！").c_str());
    } else {
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("图片处理失败").c_str());
    }
}

void picture::on_addFile_clicked()
{
    QString selectedFile = QFileDialog::getOpenFileName(this, gbk_to_utf8("选择图片文件").c_str(), QDir::currentPath(), gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp);;所有文件 (*.*)").c_str());
    if (!selectedFile.isEmpty()) {
        // 验证文件是否为有效图片
        if (!isValidImageFile(selectedFile)) {
            QMessageBox::warning(this, gbk_to_utf8("警告").c_str(), gbk_to_utf8("选择的文件不是有效的图片文件或文件损坏").c_str());
            return;
        }

        file = selectedFile;
        qDebug() << "Selected file:" << file;

        // 设置按钮文字显示文件名
        QFileInfo fileInfo(file);
        QString shortFileName = fileInfo.fileName();
        if (shortFileName.length() > 20) {
            shortFileName = shortFileName.left(17) + "...";
        }
        ui->addFile->setText(gbk_to_utf8("已选择:").c_str() + shortFileName);

        // 如果用户未选择输出路径，自动生成一个
        if (outFile.isEmpty()) {
            outFile = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_processed." + fileInfo.suffix();
        }
    } else {
        qDebug() << "file selection cancelled or empty";
    }
}

void picture::on_exportFile_clicked()
{
    QString selectedOutFile = QFileDialog::getSaveFileName(this, gbk_to_utf8("选择输出文件").c_str(), outFile.isEmpty() ? QDir::currentPath() : outFile, gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp)").c_str());
    if (!selectedOutFile.isEmpty()) {
        outFile = selectedOutFile;
        qDebug() << "Output file set to:" << outFile;

        // 设置按钮文字显示输出文件名
        QFileInfo fileInfo(outFile);
        QString shortFileName = fileInfo.fileName();
        if (shortFileName.length() > 20) {
            shortFileName = shortFileName.left(17) + "...";
        }
        ui->exportFile->setText(gbk_to_utf8("导出:").c_str() + shortFileName);
    }
}

void picture::showLoading()
{
    // 禁用界面
    setEnabled(false);
}

void picture::hideLoading()
{
    // 启用界面
    setEnabled(true);
}
