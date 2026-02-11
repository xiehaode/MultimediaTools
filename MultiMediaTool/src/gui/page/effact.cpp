#include "effact.h"
#include "ui_effact.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <QMessageBox>
#include "src/utils/lan_util.h"

effact::effact(QWidget *parent) :
    ui(new Ui::effact)
{
    ui->setupUi(this);
    initUI();
    trans = VideoTrans_Create();
    if (!trans) {
        qDebug()<<"VideoTrans_Create fair";
    }
    m_processor = AVProcessor_Create();
    if (!m_processor) {
        qDebug() << "AVProcessor_Create failed";
    }
}

effact::~effact()
{
    if (trans) {
        VideoTrans_Destroy(trans);
        trans = nullptr;
    }
    if (m_processor) {
        AVProcessor_Destroy(m_processor);
        m_processor = nullptr;
    }
    delete ui;
}

void effact::initUI()
{
    setupCheckBoxConnections();

    // 初始化格式转换下拉框
    QStringList srcFormats = {"MP4", "AVI", "MKV", "MOV", "TS"};
    QStringList dstFormats = {"MP4", "AVI", "MKV", "MOV", "GIF"};
    ui->comboBoxSrc->addItems(srcFormats);
    ui->comboBoxDst->addItems(dstFormats);

    ui->comboBoxSrc->setCurrentText("MP4");
    ui->comboBoxDst->setCurrentText("MP4");
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

void effact::on_btnImport_clicked()
{
    m_importPath = QFileDialog::getOpenFileName(this, "选择要转换的文件", "", "Video Files (*.mp4 *.avi *.mkv *.mov *.ts);;All Files (*.*)");
    if (!m_importPath.isEmpty()) {
        ui->lineEdit_Path->setText(m_importPath);

        // 自动识别源格式
        QString ext = QFileInfo(m_importPath).suffix().toUpper();
        int index = ui->comboBoxSrc->findText(ext);
        if (index != -1) {
            ui->comboBoxSrc->setCurrentIndex(index);
        }
    }
}

void effact::on_comboBoxSrc_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    // 这里可以根据源格式动态调整目标格式列表，暂不实现
}

void effact::on_pushButton_clicked()
{
    if (m_importPath.isEmpty()) {
        QMessageBox::warning(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("未选择输入文件").c_str());
        return;
    }

    QString srcFormat = ui->comboBoxSrc->currentText().toLower();
    QString dstFormat = ui->comboBoxDst->currentText().toLower();

    QString outPath = QFileDialog::getSaveFileName(this, "选择输出路径",
                                                  QDir::currentPath(),
                                                  QString("Video Files (*.%1)").arg(dstFormat));

    if (outPath.isEmpty()) return;

    ui->pushButton->setEnabled(false);

    std::string input = QDir::toNativeSeparators(m_importPath).toUtf8().constData();
    std::string output = QDir::toNativeSeparators(outPath).toUtf8().constData();

    //  核心修复：创建独立的处理器副本，避免线程竞争
    void* processor_copy = AVProcessor_Create();
    if (!processor_copy) {
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("无法创建处理器副本").c_str());
        ui->pushButton->setEnabled(true);
        return;
    }

    QFutureWatcher<int>* watcher = new QFutureWatcher<int>(this);

    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher, outPath, srcFormat, dstFormat, processor_copy]() {
        int result = watcher->result();

        //  确保异步任务完成后销毁副本
        AVProcessor_Destroy(processor_copy);

        if (result == 0) {
            QMessageBox::information(this, gbk_to_utf8("成功").c_str(), QString(gbk_to_utf8("成功").c_str()).arg(outPath));
        } else {
            QString errorMsg = QString("转换失败，错误码: %1").arg(result);
            QMessageBox::critical(this, "转换失败", errorMsg);
        }

        ui->pushButton->setEnabled(true);
        watcher->deleteLater();
    });

    //  使用独立副本进行异步操作
    QFuture<int> future = QtConcurrent::run([this, processor_copy, input, output, dstFormat]() -> int {
        try {
            if (dstFormat == "gif") {
                AVConfig config;
                config.width = 480;
                config.frame_rate = 10;
                return AVProcessor_Mp4ToGif(processor_copy, input.c_str(), output.c_str(), &config);
            } else {
                return AVProcessor_Remux(processor_copy, input.c_str(), output.c_str());
            }
        } catch (...) {
            qDebug() << "转换过程中发生异常";
            return -999;
        }
    });

    watcher->setFuture(future);
}


