#include "picture.h"
#include "ui_picture.h"
#include "src/utils/lan_util.h"
#include "src/utils/screenCapture.h"
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
    QWidget(parent),
    ui(new Ui::picture),
    isProcessing(false)
{
    ui->setupUi(this);
    initUI();

    // 设置三栏布局的stretch比例为2:2:3
    ui->horizontalLayout_6->setStretch(0, 2);
    ui->horizontalLayout_6->setStretch(1, 2);
    ui->horizontalLayout_6->setStretch(2, 3);

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
    setupInputWidgets();

    // 初始化所有参数控件为隐藏状态
    QList<QWidget*> allParamWidgets = {
        ui->spinBoxArgs1, ui->spinBoxArgs2, ui->spinBoxArgs3, ui->spinBoxArgs4, ui->spinBox_7,
        ui->doubleSpinBox, ui->lineEdit_2
    };

    for (QWidget* w : allParamWidgets) {
        if (w) {
            w->setVisible(false);
            w->setEnabled(false);
        }
    }

    // 设置按钮提示显示信息
    ui->addFile->setToolTip(gbk_to_utf8("选择要处理的图片文件").c_str());
    ui->exportFile->setToolTip(gbk_to_utf8("选择保存文件的目标位置").c_str());
    ui->ok->setToolTip(gbk_to_utf8("开始处理图片").c_str());
    ui->cancel->setToolTip(gbk_to_utf8("取消当前选择").c_str());
}

void picture::setupCheckBoxConnections()
{

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
    //connect(ui->addFile, &QAbstractButton::clicked, this, &picture::on_addFile_clicked);   //111
    //connect(ui->exportFile, &QAbstractButton::clicked, this, &picture::on_exportFile_clicked);
    //connect(ui->ok, &QAbstractButton::clicked, this, &picture::on_ok_clicked);

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

        // 隐藏所有参数控件
        QList<QWidget*> allParamWidgets = {
            ui->spinBoxArgs1, ui->spinBoxArgs2, ui->spinBoxArgs3, ui->spinBoxArgs4, ui->spinBox_7,
            ui->doubleSpinBox, ui->lineEdit_2
        };

        for (QWidget* w : allParamWidgets) {
            if (w) {
                w->setVisible(false);
                w->setEnabled(false);
            }
        }


        updateParameterWidget();
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

    // 更新参数UI显示
    updataParamUi();

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
    // 参数验证
    bool paramValid = true;
    QString errorMsg;

    if(effectType == addTextWatermark) {
        QString watermarkText = m_waterTextWidget->findChild<QLineEdit*>("lineEdit_watermarkText")->text();
        if(watermarkText.isEmpty()){
            QMessageBox::information(this, gbk_to_utf8("提示").c_str(), gbk_to_utf8("未设置水印文字").c_str());
            return;
        }
    }
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
        outFile = QFileDialog::getSaveFileName(this, gbk_to_utf8("选择输出文件").c_str(), QDir::currentPath()+"/document", gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp)").c_str());
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

    // 参考effact页面的参数结构
    param mParam;
    memset(&mParam, 0, sizeof(mParam)); // 清零至关重要

    // 从对应的input控件获取参数
    switch (effectType) {
    case grayImage:
        mParam = m_greyWidget->getParams();
        break;
    case FrostedGlass:
        mParam = m_glassWidget->getParams();
        break;
    case simpleSkinSmoothing:
        mParam = m_smoothWidget->getParams();
        break;
    case Whitening:
        mParam = m_white1Widget->getParams();
        break;
    case Whitening2:
        mParam = m_white2Widget->getParams();
        break;
    case invertImage:
        mParam = m_greyWidget->getParams();
        break;
    case customOilPaintApprox:
        mParam = m_easyOilPaintingWidget->getParams();
        break;
    case applyOilPainting:
        mParam = m_oilPaintingWidget->getParams();
        break;
    case applyMosaic:
        mParam = m_mosaicWidget->getParams();
        break;
    case addTextWatermark:
        mParam = m_waterTextWidget->getParams();
        break;
    default:
        // 其他特效不需要额外参数
        break;
    }

    QFuture<bool> future = QtConcurrent::run([this, utf8InputStr, utf8OutputStr, mParam]() -> bool {
        const char* inputPath = utf8InputStr.c_str();
        const char* outputPath = utf8OutputStr.c_str();
        switch (effectType) {
            case grayImage:
                return CvTranslator_GrayImage_File(translator, inputPath, outputPath);
            case addTextWatermark:
                return CvTranslator_AddTextWatermark_File(translator, inputPath, outputPath, mParam.arr);
            case customOilPaintApprox:
                return CvTranslator_OilPainting_File(translator, inputPath, outputPath, mParam.iparam1, (int)mParam.dparam1);
            case applyOilPainting:
                return CvTranslator_OilPainting_File(translator, inputPath, outputPath, mParam.iparam1, (int)mParam.dparam1);
            case applyMosaic:
                return CvTranslator_Mosaic_File(translator, inputPath, outputPath, mParam.iparam1, mParam.iparam2, mParam.iparam3, mParam.iparam4, mParam.iparam5);
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
    QString selectedFile = QFileDialog::getOpenFileName(this, gbk_to_utf8("选择图片文件").c_str(), QDir::currentPath()+"/document", gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp);;所有文件 (*.*)").c_str());
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
    QString selectedOutFile = QFileDialog::getSaveFileName(this, gbk_to_utf8("选择输出文件").c_str(), outFile.isEmpty() ? QDir::currentPath()+"/document" : outFile, gbk_to_utf8("图片文件 (*.png *.jpg *.jpeg *.bmp)").c_str());
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

void picture::updataParamUi()
{
    updateParameterWidget();
}

void picture::setupInputWidgets()
{
    // 创建所有input控件实例
    m_glassWidget = new glass(this);
    m_greyWidget = new grey(this);
    m_mosaicWidget = new mosaic(this);
    m_oilPaintingWidget = new oilPainting(this);
    m_reverseWidget = new reverse(this);
    m_smoothWidget = new smooth(this);
    m_white1Widget = new white1(this);
    m_white2Widget = new white2(this);
    m_waterTextWidget = new waterText(this);
    m_easyOilPaintingWidget = new easyOilPainting(this);

    // 将所有控件添加到verticalLayout_2布局中
    ui->verticalLayout_2->addWidget(m_glassWidget);
    ui->verticalLayout_2->addWidget(m_greyWidget);
    ui->verticalLayout_2->addWidget(m_mosaicWidget);
    ui->verticalLayout_2->addWidget(m_oilPaintingWidget);
    ui->verticalLayout_2->addWidget(m_reverseWidget);
    ui->verticalLayout_2->addWidget(m_smoothWidget);
    ui->verticalLayout_2->addWidget(m_white1Widget);
    ui->verticalLayout_2->addWidget(m_white2Widget);
    ui->verticalLayout_2->addWidget(m_waterTextWidget);
    ui->verticalLayout_2->addWidget(m_easyOilPaintingWidget);

    // 初始状态隐藏所有控件
    m_glassWidget->hide();
    m_greyWidget->hide();
    m_mosaicWidget->hide();
    m_oilPaintingWidget->hide();
    m_reverseWidget->hide();
    m_smoothWidget->hide();
    m_white1Widget->hide();
    m_white2Widget->hide();
    m_waterTextWidget->hide();
    m_easyOilPaintingWidget->hide();
}

void picture::updateParameterWidget()
{
    // 隐藏所有控件
    m_glassWidget->hide();
    m_greyWidget->hide();
    m_mosaicWidget->hide();
    m_oilPaintingWidget->hide();
    m_reverseWidget->hide();
    m_smoothWidget->hide();
    m_white1Widget->hide();
    m_white2Widget->hide();
    m_waterTextWidget->hide();
    m_easyOilPaintingWidget->hide();

    // 根据effectType显示对应的控件
    switch (effectType) {
    case grayImage:
        m_greyWidget->show();
        break;
    case FrostedGlass:
        m_glassWidget->show();
        break;
    case simpleSkinSmoothing:
        m_smoothWidget->show();
        break;
    case Whitening:
        m_white1Widget->show();
        break;
    case Whitening2:
        m_white2Widget->show();
        break;
    case invertImage:
        m_reverseWidget->show();
        break;
    case customOilPaintApprox:
        m_easyOilPaintingWidget->show();
        break;
    case applyOilPainting:
        m_oilPaintingWidget->show();
        break;
    case applyMosaic:
        m_mosaicWidget->show();
        break;
    case addTextWatermark:
        m_waterTextWidget->show();
        break;
    default:
        break;
    }
}

void picture::on_pushButton_clicked()
{
    QString defaultDir = QDir::currentPath() + "/document";
    QString defaultFilePath = defaultDir + "/screenshot.bmp";


    QString savePath = QFileDialog::getSaveFileName(
        this,
        gbk_to_utf8("保存截屏图片").c_str(),
        defaultFilePath,  // 默认保存路径+文件名
        gbk_to_utf8("位图文件 (*.bmp);;所有文件 (*.*)").c_str()
    );


    qDebug()<<"保存位置"<<savePath;
    try {
        bool success = QuickCaptureScreen(savePath.toStdString());
        if (success) {
            QMessageBox::information(this, gbk_to_utf8("成功").c_str(),
                gbk_to_utf8("截屏已保存至：").c_str() + savePath);
        } else {
            QMessageBox::critical(this, gbk_to_utf8("错误").c_str(),
                gbk_to_utf8("截屏保存失败，底层捕获函数返回错误").c_str());
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(),
            QString(gbk_to_utf8("截屏保存失败：").c_str()) + e.what());
        qDebug() << "截屏失败原因：" << e.what();
    } catch (...) {
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(),
            gbk_to_utf8("截屏保存失败，未知错误").c_str());
    }
}
