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

// 声明GBK转UTF8函数（确保项目中有该函数的实现）
std::string gbk_to_utf8(const std::string& gbk_str);

concat::concat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::concat),
    m_worker(nullptr)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(3);
    // 设置表格表头（修复中文乱码）
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

    connect(ui->btn_Refresh, &QPushButton::clicked, this, &concat::on_btn_Refresh_clicked);
    connect(ui->btn_Merge, &QPushButton::clicked, this, &concat::on_btn_Merge_clicked);
    connect(ui->btn_Split, &QPushButton::clicked, this, &concat::on_btn_Split_clicked);
    connect(ui->btn_Resize, &QPushButton::clicked, this, &concat::on_btn_Resize_clicked);
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &concat::on_tableWidget_itemChanged);
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
    dir.cdUp();
    dir.cd("video");
    return dir.absolutePath();
}

void concat::loadVideoFiles()
{
    ui->tableWidget->setRowCount(0);
    m_videoFiles.clear();

    QString videoDir = getVideoDir();
    QDir dir(videoDir);

    if (!dir.exists()) {
        qDebug() << "Video directory does not exist:" << videoDir;
        return;
    }

    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv" << "*.flv" << "*.mov" << "*.wmv" << "*.ts";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    ui->tableWidget->setRowCount(fileList.size());

    for (int i = 0; i < fileList.size(); ++i) {
        QFileInfo fileInfo = fileList.at(i);
        m_videoFiles.append(fileInfo.absoluteFilePath());

        QTableWidgetItem* checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(i, 0, checkItem);

        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(fileInfo.fileName()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(fileInfo.absoluteFilePath()));
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
            // 修复提示框中文乱码
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

    // 修复文件保存对话框中文
    QString outputFile = QFileDialog::getSaveFileName(this,
        QString::fromUtf8(gbk_to_utf8("保存合并后的视频").c_str()),
        getVideoDir() + "/merged.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    bool result = AvWorker_SpliceAV(m_worker,
                                     selected.at(0).toUtf8().constData(),
                                     selected.at(1).toUtf8().constData(),
                                     outputFile.toUtf8().constData(),
                                     false);

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频合并成功！").c_str()));
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

    bool ok;
    // 修复输入对话框中文
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
        QString::fromUtf8(gbk_to_utf8("保存分割后的视频").c_str()),
        getVideoDir() + "/split.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    bool result = AvWorker_split_video(m_worker,
                                       selected.at(0).toUtf8().constData(),
                                       outputFile.toUtf8().constData(),
                                       startTime,
                                       duration);

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频分割成功！").c_str()));
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

    bool ok;
    // 修复输入对话框中文
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
        QString::fromUtf8(gbk_to_utf8("保存调整后的视频").c_str()),
        getVideoDir() + "/resized.mp4",
        QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv)").c_str()));
    if (outputFile.isEmpty()) {
        return;
    }

    bool result = AvWorker_resize_video(m_worker,
                                        selected.at(0).toUtf8().constData(),
                                        outputFile.toUtf8().constData(),
                                        width,
                                        height);

    if (result) {
        QMessageBox::information(this,
            QString::fromUtf8(gbk_to_utf8("成功").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频调整大小成功！").c_str()));
    } else {
        QMessageBox::critical(this,
            QString::fromUtf8(gbk_to_utf8("失败").c_str()),
            QString::fromUtf8(gbk_to_utf8("视频调整大小失败！").c_str()));
    }
}
