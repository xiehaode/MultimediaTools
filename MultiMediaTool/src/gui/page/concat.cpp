#include "concat.h"
#include "ui_concat.h"
#include "OpenCVFFMpegTools.h"

#include <QDir>
#include <QFileInfo>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDebug>

// 声明GBK转UTF8函数(请确保你的项目中有该函数实现)
std::string gbk_to_utf8(const std::string& gbk_str);

concat::concat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::concat),
    m_worker(nullptr),
    m_progressWidget(nullptr)
{
    ui->setupUi(this);

    // 初始化进度显示组件
    m_progressWidget = new QWidget(this);
    m_progressWidget->setVisible(false);
    
    QHBoxLayout* progressLayout = new QHBoxLayout(m_progressWidget);
    m_progressLabel = new QLabel("处理中...");
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0); // 无限进度条
    
    progressLayout->addWidget(m_progressLabel);
    progressLayout->addWidget(m_progressBar);
    progressLayout->setContentsMargins(10, 5, 10, 5);
    
    ui->verticalLayout->insertWidget(2, m_progressWidget);

    ui->tableWidget->setColumnCount(3);
    // 设置表头(无需修改，已修复)
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
        << QString::fromUtf8(gbk_to_utf8("选择").c_str())
        << QString::fromUtf8(gbk_to_utf8("文件名").c_str())
        << QString::fromUtf8(gbk_to_utf8("路径").c_str()));

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_worker = AvWorker_Create();
    if (!m_worker) {
        qDebug() << "AvWorker_Create failed!";
    }

    loadVideoFiles();

    //connect(ui->btn_Refresh, &QPushButton::clicked, this, &concat::on_btn_Refresh_clicked);
    //connect(ui->btn_Merge, &QPushButton::clicked, this, &concat::on_btn_Merge_clicked);
    //connect(ui->btn_Split, &QPushButton::clicked, this, &concat::on_btn_Split_clicked);
    //connect(ui->btn_Resize, &QPushButton::clicked, this, &concat::on_btn_Resize_clicked);
    //connect(ui->tableWidget, &QTableWidget::itemChanged, this, &concat::on_tableWidget_itemChanged);
}

concat::~concat()
{
    if (m_worker) {
        AvWorker_Destroy(m_worker);
        m_worker = nullptr;
    }
    delete ui;
}

QString concat::getVideoDir()
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cd("video");
    return dir.absolutePath();
}

void concat::loadVideoFiles()
{
    ui->tableWidget->setRowCount(0);
    m_videoFiles.clear();

    QString videoDir = getVideoDir();
    qDebug() << "Loading videos from directory:" << videoDir;
    QDir dir(videoDir);

    if (!dir.exists()) {
        // 尝试创建目录
        if (!dir.mkpath(videoDir)) {
            QMessageBox::warning(this,
                QString::fromUtf8(gbk_to_utf8("警告").c_str()),
                QString::fromUtf8(gbk_to_utf8("视频目录不存在且无法创建：").c_str()) + videoDir);
            return;
        }
    }

    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv" << "*.flv" << "*.mov" << "*.wmv" << "*.ts";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    ui->tableWidget->setRowCount(fileList.size());

    int validFileIndex = 0;
    for (int i = 0; i < fileList.size(); ++i) {
        QFileInfo fileInfo = fileList.at(i);
        
        // 简单验证文件是否可读，不显示错误消息
        if (fileInfo.exists() && fileInfo.isReadable()) {
            QString suffix = fileInfo.suffix().toLower();
            QStringList validSuffixes = {"mp4", "avi", "mkv", "flv", "mov", "wmv", "ts"};
            
            if (validSuffixes.contains(suffix)) {
                m_videoFiles.append(fileInfo.absoluteFilePath());

                QTableWidgetItem* checkItem = new QTableWidgetItem();
                checkItem->setCheckState(Qt::Unchecked);
                ui->tableWidget->setItem(validFileIndex, 0, checkItem);

                // 显示文件名和大小
                QString fileName = fileInfo.fileName();
                QString fileSize = QString::number(fileInfo.size() / (1024.0 * 1024.0), 'f', 2) + " MB";
                QString displayText = fileName + " (" + fileSize + ")";
                
                ui->tableWidget->setItem(validFileIndex, 1, new QTableWidgetItem(displayText));
                ui->tableWidget->setItem(validFileIndex, 2, new QTableWidgetItem(fileInfo.absoluteFilePath()));
                
                validFileIndex++;
            }
        }
    }

    // 调整表格行数以匹配实际加载的文件数
    ui->tableWidget->setRowCount(m_videoFiles.size());

    if (m_videoFiles.isEmpty()) {
        QTableWidgetItem* hintItem = new QTableWidgetItem(QString::fromUtf8(gbk_to_utf8("未找到有效的视频文件").c_str()));
        hintItem->setFlags(hintItem->flags() & ~Qt::ItemIsSelectable);
        hintItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, hintItem);
        ui->tableWidget->setSpan(0, 0, 1, 3);
    }

    updateButtonStates();
}

QStringList concat::getSelectedVideos()
{
    QStringList selected;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem* item = ui->tableWidget->item(i, 0);
        if (item && item->checkState() == Qt::Checked) {
            selected.append(m_videoFiles.at(i));
        }
    }
    return selected;
}

void concat::updateButtonStates()
{
    QStringList selected = getSelectedVideos();
    int count = selected.size();

    ui->btn_Merge->setEnabled(count == 2);
    ui->btn_Split->setEnabled(count == 1);
    ui->btn_Resize->setEnabled(count == 1);
}

void concat::on_btn_Refresh_clicked()
{
    loadVideoFiles();
}

void concat::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        QStringList selected = getSelectedVideos();
        if (selected.size() > 2) {
            item->setCheckState(Qt::Unchecked);
            // 修复显示提示信息
            QMessageBox::warning(this,
                QString::fromUtf8(gbk_to_utf8("提示").c_str()),
                QString::fromUtf8(gbk_to_utf8("最多只能选择2个视频！").c_str()));
            return;
        }
        updateButtonStates();
    }
}

void concat::on_btn_Merge_clicked()
{
    QStringList selected = getSelectedVideos();
    if (selected.size() != 2) {
        QMessageBox::warning(this,
            QString::fromUtf8(gbk_to_utf8("提示").c_str()),
            QString::fromUtf8(gbk_to_utf8("请选择2个视频进行合并！").c_str()));
        return;
    }

    // 验证选择的文件
    for (const QString& file : selected) {
        if (!validateVideoFile(file)) {
            return;
        }
    }

    QString outputFile = QFileDialog::getSaveFileName(this,
        QString::fromUtf8(gbk_to_utf8("选择合并后的视频").c_str()),
        getVideoDir() + "/merged.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    showProgress(QString::fromUtf8(gbk_to_utf8("正在合并视频...").c_str()));

    bool result = AvWorker_SpliceAV(m_worker,
                                     selected.at(0).toUtf8().constData(),
                                     selected.at(1).toUtf8().constData(),
                                     outputFile.toUtf8().constData(),
                                     false);

    hideProgress();

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频合并成功！").c_str()));
        loadVideoFiles(); // 刷新文件列表
    } else {
        QMessageBox::critical(this,
            QString::fromUtf8(gbk_to_utf8("失败").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频合并失败！").c_str()));
    }
}

void concat::on_btn_Split_clicked()
{
    QStringList selected = getSelectedVideos();
    if (selected.size() != 1) {
        QMessageBox::warning(this,
            QString::fromUtf8(gbk_to_utf8("提示").c_str()),
            QString::fromUtf8(gbk_to_utf8("请选择1个视频进行分割！").c_str()));
        return;
    }

    // 验证选择的文件
    if (!validateVideoFile(selected.first())) {
        return;
    }

    bool ok;
    double startTime = QInputDialog::getDouble(this,
        QString::fromUtf8(gbk_to_utf8("分割视频").c_str()),
        QString::fromUtf8(gbk_to_utf8("请输入开始时间(秒)：").c_str()),
        0, 0, 999999, 2, &ok);
    if (!ok) return;

    double duration = QInputDialog::getDouble(this,
        QString::fromUtf8(gbk_to_utf8("分割视频").c_str()),
        QString::fromUtf8(gbk_to_utf8("请输入持续时间(秒)：").c_str()),
        10, 1, 999999, 2, &ok);
    if (!ok) return;

    QString outputFile = QFileDialog::getSaveFileName(this,
        QString::fromUtf8(gbk_to_utf8("选择分割后的视频").c_str()),
        getVideoDir() + "/split.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    showProgress(QString::fromUtf8(gbk_to_utf8("正在分割视频...").c_str()));

    bool result = AvWorker_split_video(m_worker,
                                       selected.at(0).toUtf8().constData(),
                                       outputFile.toUtf8().constData(),
                                       startTime,
                                       duration);

    hideProgress();

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频分割成功！").c_str()));
        loadVideoFiles(); // 刷新文件列表
    } else {
        QMessageBox::critical(this,
            QString::fromUtf8(gbk_to_utf8("失败").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频分割失败！").c_str()));
    }
}

void concat::on_btn_Resize_clicked()
{
    QStringList selected = getSelectedVideos();
    if (selected.size() != 1) {
        QMessageBox::warning(this,
            QString::fromUtf8(gbk_to_utf8("提示").c_str()),
            QString::fromUtf8(gbk_to_utf8("请选择1个视频调整大小！").c_str()));
        return;
    }

    // 验证选择的文件
    if (!validateVideoFile(selected.first())) {
        return;
    }

    bool ok;
    int width = QInputDialog::getInt(this,
        QString::fromUtf8(gbk_to_utf8("调整视频大小").c_str()),
        QString::fromUtf8(gbk_to_utf8("请输入目标宽度：").c_str()),
        1920, 1, 7680, 1, &ok);
    if (!ok) return;

    int height = QInputDialog::getInt(this,
        QString::fromUtf8(gbk_to_utf8("调整视频大小").c_str()),
        QString::fromUtf8(gbk_to_utf8("请输入目标高度：").c_str()),
        1080, 1, 4320, 1, &ok);
    if (!ok) return;

    QString outputFile = QFileDialog::getSaveFileName(this,
        QString::fromUtf8(gbk_to_utf8("选择调整后的视频").c_str()),
        getVideoDir() + "/resized.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    showProgress(QString::fromUtf8(gbk_to_utf8("正在调整视频大小...").c_str()));

    bool result = AvWorker_resize_video(m_worker,
                                        selected.at(0).toUtf8().constData(),
                                        outputFile.toUtf8().constData(),
                                        width,
                                        height);

    hideProgress();

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频调整大小成功！").c_str()));
        loadVideoFiles(); // 刷新文件列表
    } else {
        QMessageBox::critical(this,
            QString::fromUtf8(gbk_to_utf8("失败").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频调整大小失败！").c_str()));
    }
}

void concat::showProgress(const QString& title)
{
    m_progressLabel->setText(title);
    m_progressWidget->setVisible(true);
    // 禁用所有按钮
    ui->btn_Refresh->setEnabled(false);
    ui->btn_Merge->setEnabled(false);
    ui->btn_Split->setEnabled(false);
    ui->btn_Resize->setEnabled(false);
    ui->tableWidget->setEnabled(false);
    QCoreApplication::processEvents();
}

void concat::hideProgress()
{
    m_progressWidget->setVisible(false);
    // 重新启用按钮和表格
    updateButtonStates();
    ui->tableWidget->setEnabled(true);
}

bool concat::validateVideoFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        QMessageBox::warning(this,
            QString::fromUtf8(gbk_to_utf8("错误").c_str()),
            QString::fromUtf8(gbk_to_utf8("文件不存在或不可读：").c_str()) + filePath);
        return false;
    }
    
    // 检查文件扩展名
    QString suffix = fileInfo.suffix().toLower();
    QStringList validSuffixes = {"mp4", "avi", "mkv", "flv", "mov", "wmv", "ts"};
    if (!validSuffixes.contains(suffix)) {
        QMessageBox::warning(this,
            QString::fromUtf8(gbk_to_utf8("错误").c_str()),
            QString::fromUtf8(gbk_to_utf8("不支持的视频格式：").c_str()) + suffix);
        return false;
    }
    
    return true;
}
