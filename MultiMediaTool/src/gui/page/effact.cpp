#include "effact.h"
#include "ui_effact.h"
#include "src/utils/lan_util.h"
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <QMessageBox>
#include <cstring>

effact::effact(QWidget *parent) :
    QWidget(parent),
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
    //设置拖拽
    m_dragHandler = new DragDropHandler(this);
    m_dragHandler->attach(this);  // 绑定到当前窗口

    // 设置文件拖拽回调
    m_dragHandler->setOnFileDropped([this](const QList<QString> &filePaths) {
        this->handleDroppedFiles(filePaths);
    });

    QList<QWidget*> allParamWidgets = {
        ui->spinBoxArgs1, ui->spinBoxArgs2, ui->spinBoxArgs3, ui->spinBoxArgs4, ui->spinBoxArgs5,
        ui->doubleSpinBoxArgs1,
        ui->argLine1,
    };

    for (QWidget* w : allParamWidgets) {
        if (w) {
            w->setVisible(false);
            w->setEnabled(false);
        }
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

    ui->addFile->setText(gbk_to_utf8("待处理文件").c_str());
    ui->addFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // 文字在图标下方
    ui->exportFile->setText(gbk_to_utf8("输出文件").c_str());
    ui->exportFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // 文字在图标下方
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
    if(s == setNone){
        s = setFile1;
    }
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
    updataParamUi();
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

void effact::handleDroppedFiles(const QList<QString> &filePaths)
{
    if(s == setNone){
        return;
    }
    else if(s == setFile1){
        if(filePaths.size()>0){
            file = filePaths[0];
            QFileInfo fileInfo(file);
            QString shortFileName = fileInfo.fileName();
            if (shortFileName.length() > 20) {
                shortFileName = shortFileName.left(17) + "...";
            }
            ui->addFile->setText(shortFileName);
            on_exportFile_clicked();
            s = finished;
        }

    }
    else if(s == setFile2){
        return;
    }
    else if(s == finished){
        return;
    }
    else{
        return;
    }
}

void effact::on_ok_clicked()
{
    if (s != finished) {
        QMessageBox::information(this, gbk_to_utf8("提示").c_str(), gbk_to_utf8("请先选择输入和输出文件并完成初始化").c_str());
        return;
    }

    if (!trans) {
        qDebug() << "视频转换器未初始化";
        QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), gbk_to_utf8("转换器未初始化，请重试").c_str());
        return;
    }

    this->setEnabled(false);
    ui->ok->setEnabled(false);

    qDebug() << "开始处理视频... 效果类型:" << effectType;

    param mParam;
    memset(&mParam, 0, sizeof(mParam)); // 清零至关重要

    bool paramValid = true;
    QString errorMsg;

    switch (effectType) {
    case grayImage:
    case FrostedGlass:
    case simpleSkinSmoothing:
    case Whitening:
    case Whitening2:
    case invertImage:{
        // 这些特效不需要额外参数
        break;
    }
    case customOilPaintApprox:
    case applyOilPainting:{

        mParam.iparam1 = ui->spinBoxArgs1->value(); // 细节
        mParam.dparam1 = ui->doubleSpinBoxArgs1->value(); // 强度

        if (mParam.iparam1 <= 0) { errorMsg = "油画细节必须大于 0"; paramValid = false; }
        if (mParam.dparam1 <= 0.0) { errorMsg = "油画强度必须大于 0"; paramValid = false; }
        break;
    }

    case applyMosaic:{

        mParam.iparam1 = ui->spinBoxArgs1->value(); // x
        mParam.iparam2 = ui->spinBoxArgs2->value(); // y
        mParam.iparam3 = ui->spinBoxArgs3->value(); // w
        mParam.iparam4 = ui->spinBoxArgs4->value(); // h
        mParam.iparam5 = ui->spinBoxArgs5->value(); // 块大小

        if (mParam.iparam5 <= 0) { errorMsg = "马赛克块大小必须大于 0"; paramValid = false; }
        break;
    }
    case addTextWatermark: {

        QString text = ui->argLine1->text();
        if (text.isEmpty()) {
            errorMsg = "水印文字不能为空";
            paramValid = false;
        } else {
            QByteArray ba = text.toUtf8();
            strncpy_s(mParam.arr, ba.constData(), sizeof(mParam.arr) - 1);
            mParam.arr[sizeof(mParam.arr) - 1] = '\0';
        }

        mParam.iparam1 = ui->spinBoxArgs1->value(); // x
        mParam.iparam2 = ui->spinBoxArgs2->value(); // y
        break;
    }

    default:
        break;
    }

    if (!paramValid) {
        this->setEnabled(true);
        ui->ok->setEnabled(true);

        QMessageBox::warning(this, gbk_to_utf8("参数错误").c_str(), gbk_to_utf8(errorMsg.toStdString()).c_str());
        return;
    }


    int processRet = VideoTrans_Process(trans, effectType, mParam);
    QString("").arg(15);
    if (processRet != 0) {
        qDebug() << "视频处理失败，错误码:" << processRet;
        QMessageBox::critical(this, gbk_to_utf8("处理失败").c_str(),
                              QString(gbk_to_utf8("错误码: %1").c_str()).arg(processRet));
        VideoTrans_Destroy(trans);
        trans = nullptr;
    } else {
        qDebug() << "视频处理完成！";
        QMessageBox::information(this, gbk_to_utf8("成功").c_str(), gbk_to_utf8("视频处理完成！").c_str());
        VideoTrans_Destroy(trans);
        trans = nullptr;
    }

    ui->ok->setEnabled(true);
    this->setEnabled(true);

    s = setFile1;

    ui->addFile->setText(gbk_to_utf8("待处理文件").c_str());
    ui->exportFile->setText(gbk_to_utf8("输出文件").c_str());
    file.clear();
    outFile.clear();
}

void effact::on_addFile_clicked()
{
    if(s != setFile1){
        return;
    }
    file = QFileDialog::getOpenFileName(this, gbk_to_utf8("选择视频文件").c_str(), QDir::currentPath()+"/video", gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv *.mov *.ts);;所有文件 (*.*)").c_str());
    if (file.isEmpty()) {
        qDebug()<<"未选择任何文件";
    }
    // 设置按钮文字显示输出文件名
    QFileInfo fileInfo(file);
    QString shortFileName = fileInfo.fileName();
    if (shortFileName.length() > 20) {
        shortFileName = shortFileName.left(17) + "...";
    }
    ui->addFile->setText(shortFileName);
    s = setFile2;
}

void effact::on_exportFile_clicked()
{
    if(s != setFile2){
        return;
    }
    if (file.isEmpty()) {
        qDebug() << "未选择输入文件";
        return;
    }

    outFile = QFileDialog::getSaveFileName(this, gbk_to_utf8("选择输出文件").c_str(), QDir::currentPath()+"/video", gbk_to_utf8("视频文件 (*.mp4)").c_str());

    if (outFile.isEmpty()) {
        qDebug() << "未选择输出文件";
        return;
    }

    // 转换为本地路径，确保使用UTF-8编码传递给底层C接口,防止乱码
    std::string utf8Input = QDir::toNativeSeparators(file).toUtf8().constData();
    std::string utf8Output = QDir::toNativeSeparators(outFile).toUtf8().constData();

    if (!trans) {
        trans = VideoTrans_Create();
    }

    if (!trans) {
        qDebug() << "无法创建视频转换器实例";
        return;
    }

    // 在子线程中初始化转换器,避免UI线程阻塞
    QFuture<int> future = QtConcurrent::run([this, utf8Input, utf8Output]() -> int {
        return VideoTrans_Initialize(trans, utf8Input.c_str(), utf8Output.c_str());
    });

    // 等待初始化完成
    int initRet = future.result();

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
    // 设置按钮文字显示输出文件名
    QFileInfo fileInfo(outFile);
    QString shortFileName = fileInfo.fileName();
    if (shortFileName.length() > 20) {
        shortFileName = shortFileName.left(17) + "...";
    }
    ui->exportFile->setText(shortFileName);
    s = finished;
}

void effact::on_btnImport_clicked()
{
    m_importPath = QFileDialog::getOpenFileName(this, gbk_to_utf8("选择要转换的文件").c_str(), QDir::currentPath()+"/video", gbk_to_utf8("Video Files (*.mp4 *.avi *.mkv *.mov *.ts);;All Files (*.*)").c_str());
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
    // TODO
}

void effact::on_pushButton_clicked()
{
    this->setEnabled(false);
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

    // 修改为创建新的处理器实例避免多线程冲突
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
        this->setEnabled(true);
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

void effact::updataParamUi()
{
    QList<QWidget*> allParamWidgets = {
        ui->spinBoxArgs1, ui->spinBoxArgs2, ui->spinBoxArgs3, ui->spinBoxArgs4, ui->spinBoxArgs5,
        ui->doubleSpinBoxArgs1,
        ui->argLine1,
    };

    for (QWidget* w : allParamWidgets) {
        if (w) {
            w->setVisible(false);
            w->setEnabled(false);
        }
    }

    bool paramValid = true;
    QString errorMsg;

    switch (effectType) {
    case grayImage:
    case FrostedGlass:
    case simpleSkinSmoothing:
    case Whitening:
    case Whitening2:
    case invertImage:
        break;

    case customOilPaintApprox:
    case applyOilPainting:{
        ui->spinBoxArgs1->setVisible(true);
        ui->doubleSpinBoxArgs1->setVisible(true);
        ui->spinBoxArgs1->setEnabled(true);
        ui->doubleSpinBoxArgs1->setEnabled(true);

        if(ui->spinBoxArgs1) ui->spinBoxArgs1->setToolTip(gbk_to_utf8("油画细节程度 (1-50)").c_str());
        if(ui->doubleSpinBoxArgs1) ui->doubleSpinBoxArgs1->setToolTip(gbk_to_utf8("油画强度 (0.1-10.0)").c_str());
        break;
    }
    case applyMosaic:{
        if(ui->spinBoxArgs1) ui->spinBoxArgs1->setVisible(true); // x
        if(ui->spinBoxArgs2) ui->spinBoxArgs2->setVisible(true); // y
        if(ui->spinBoxArgs3) ui->spinBoxArgs3->setVisible(true); // w
        if(ui->spinBoxArgs4) ui->spinBoxArgs4->setVisible(true); // h
        if(ui->spinBoxArgs5) ui->spinBoxArgs5->setVisible(true); // 块大小
        if(ui->spinBoxArgs1) ui->spinBoxArgs1->setEnabled(true); // x
        if(ui->spinBoxArgs2) ui->spinBoxArgs2->setEnabled(true); // y
        if(ui->spinBoxArgs3) ui->spinBoxArgs3->setEnabled(true); // w
        if(ui->spinBoxArgs4) ui->spinBoxArgs4->setEnabled(true); // h
        if(ui->spinBoxArgs5) ui->spinBoxArgs5->setEnabled(true); // 块大小
        ui->spinBoxArgs1->setToolTip("x");
        ui->spinBoxArgs2->setToolTip("y");
        ui->spinBoxArgs3->setToolTip("w");
        ui->spinBoxArgs4->setToolTip("h");
        ui->spinBoxArgs5->setToolTip("块大小");
        break;
    }
    case addTextWatermark:{
        if(ui->argLine1) {ui->argLine1->setVisible(true);ui->argLine1->setEnabled(true);}
        if(ui->spinBoxArgs1) {ui->spinBoxArgs1->setVisible(true); ui->spinBoxArgs1->setEnabled(true);}          // x
        if(ui->spinBoxArgs2) {ui->spinBoxArgs2->setVisible(true); ui->spinBoxArgs2->setEnabled(true);} // y

        if(ui->argLine1) {
            ui->argLine1->setPlaceholderText(gbk_to_utf8("请输入水印文字").c_str());
            ui->argLine1->setToolTip("水印文字内容");
        }
        
        // 设置水印坐标的默认值
        if(ui->spinBoxArgs1) {
            ui->spinBoxArgs1->setMinimum(0);
            ui->spinBoxArgs1->setMaximum(9999);
            ui->spinBoxArgs1->setValue(50); // 默认x坐标
            ui->spinBoxArgs1->setToolTip(gbk_to_utf8("水印X坐标").c_str());
        }
        if(ui->spinBoxArgs2) {
            ui->spinBoxArgs2->setMinimum(0);
            ui->spinBoxArgs2->setMaximum(9999);
            ui->spinBoxArgs2->setValue(50); // 默认y坐标
            ui->spinBoxArgs2->setToolTip(gbk_to_utf8("水印Y坐标").c_str());
        }
        break;
    }
    default:
        break;
    }


    if (!paramValid) {
        QMessageBox::warning(this, gbk_to_utf8("参数错误").c_str(), errorMsg);
        return; // 终止后续处理
    }
}
