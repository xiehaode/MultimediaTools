#include "effact.h"
#include "ui_effact.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <QMessageBox>
#include "src/utils/lan_util.h"

effact::effact(QWidget *parent) :
    QWidget(parent),  // 补充父类初始化（原代码遗漏）
    ui(new Ui::effact)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //mainLayout->addWidget(ui->centralWidget);
    this->setLayout(mainLayout);

    initUI();
    trans = VideoTrans_Create();
    if (!trans) {
        qDebug()<<"VideoTrans_Create failed";
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

    // 初始化格式转换下拉列表
    QStringList srcFormats = {"MP4", "AVI", "MKV", "MOV", "TS"};
    QStringList dstFormats = {"MP4", "AVI", "MKV", "MOV", "GIF"};
    ui->comboBoxSrc->addItems(srcFormats);
    ui->comboBoxDst->addItems(dstFormats);

    ui->comboBoxSrc->setCurrentText("MP4");
    ui->comboBoxDst->setCurrentText("MP4");
}

void effact::setupCheckBoxConnections()
{
    // 将所有复选框的clicked信号绑定到同一个处理函数
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

        // 根据选中的复选框确定对应的特效类型枚举值
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
        qDebug() << "视频转换器未初始化，请先点击'导出文件'选择路径！";
        return;
    }

    // 禁用按钮防止重复点击
    ui->ok->setEnabled(false);

    qDebug() << "开始处理视频...";
    int processRet = VideoTrans_Process(trans, effectType);

    if (processRet != 0) {
        qDebug() << "视频处理失败，错误码:" << processRet;
        // 处理失败时，销毁转换器并重置状态（避免重复使用错误实例）
        VideoTrans_Destroy(trans);
        trans = nullptr;
    } else {
        qDebug() << "视频处理完成！";
        // 处理成功后重置转换器状态，以便下次直接使用
        VideoTrans_Reset(trans);
    }

    ui->ok->setEnabled(true);
}

void effact::on_addFile_clicked()
{
    file = QFileDialog::getOpenFileName(this, "选择视频文件", "", "视频文件 (*.mp4 *.avi *.mkv *.mov *.ts);;所有文件 (*.*)");
    if (file.isEmpty()) {
        qDebug()<<"未选择任何文件";
    }
}

void effact::on_exportFile_clicked()
{
    if (file.isEmpty()) {
        qDebug() << "未选择输入文件";
        return;
    }

    // 使用getSaveFileName让用户指定文件保存位置，拼接默认目录
    outFile = QFileDialog::getSaveFileName(this, "选择输出文件", QDir::currentPath(), "视频文件 (*.mp4)");

    if (outFile.isEmpty()) {
        qDebug() << "未选择输出文件";
        return;
    }

    // 转换为本地路径，确保使用UTF-8编码传递给底层C接口
    // 适配底层库（如FFmpeg）在Windows下对UTF-8路径的支持
    std::string utf8Input = QDir::toNativeSeparators(file).toUtf8().constData();
    std::string utf8Output = QDir::toNativeSeparators(outFile).toUtf8().constData();

    if (!trans) {
        trans = VideoTrans_Create();
    }

    if (!trans) {
        qDebug() << "无法创建视频转换器实例";
        return;
    }

    // 在子线程中初始化转换器（避免UI线程阻塞）
    QFuture<int> future = QtConcurrent::run([this, utf8Input, utf8Output]() -> int {
        return VideoTrans_Initialize(trans, utf8Input.c_str(), utf8Output.c_str());
    });

    // 等待初始化完成
    int initRet = future.result();

    // 如果返回值为特定错误码(-4)，可能是获取宽高失败，可提示用户需要设置视频宽高或直接跳过
    // 注意：当前底层逻辑在Initialize失败时不会释放资源，需手动处理
    if (initRet != 0) {
        qDebug() << "初始化失败，错误码:" << initRet;
        if (initRet == -4) {
            qDebug() << "提示：可能需要设置视频的宽高参数，若获取失败可尝试手动设置或跳过该视频";
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
    // 可根据修改的源格式动态调整目标格式列表（暂未实现）
}

void effact::on_pushButton_clicked()
{
    if (m_importPath.isEmpty()) {
        QMessageBox::warning(this, gbk_to_utf8("警告").c_str(), gbk_to_utf8("未选择输入文件").c_str());
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

    // 注意：修改为创建新的处理器实例避免多线程冲突
    void* processor_copy = AVProcessor_Create();
    if (!processor_copy) {
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("无法创建处理器实例").c_str());
        ui->pushButton->setEnabled(true);
        return;
    }

    QFutureWatcher<int>* watcher = new QFutureWatcher<int>(this);

    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher, outPath, srcFormat, dstFormat, processor_copy]() {
        int result = watcher->result();

        // 确保异步操作完成后释放资源
        AVProcessor_Destroy(processor_copy);

        if (result == 0) {
            QMessageBox::information(this, gbk_to_utf8("成功").c_str(), QString(gbk_to_utf8("转换成功：%1").c_str()).arg(outPath));
        } else {
            QString errorMsg = QString("转换失败，错误码: %1").arg(result);
            QMessageBox::critical(this, "转换失败", errorMsg);
        }

        ui->pushButton->setEnabled(true);
        watcher->deleteLater();
    });

    // 使用新的处理器实例进行异步转换
    QFuture<int> future = QtConcurrent::run([processor_copy, input, output, dstFormat]() -> int {
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
