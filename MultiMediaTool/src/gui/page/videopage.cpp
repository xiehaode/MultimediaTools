#include <src/utils/encodinghelper.h>
#include "videopage.h"
#include "ui_videopage.h"
#include <QString>
#include <QStyle>

#include <QDir>
#include <QFileInfo>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDateTime>
#include <QSize>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProcess>
#include <QRegularExpression>

videoPage::videoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::videoPage)
{
    ui->setupUi(this);
    init();
    // 初始化表格（必须在构造函数中调用，确保页面加载时就执行）
    initableWidget();
    worker = AvWorker_Create();
}

videoPage::~videoPage()
{
    AvWorker_Destroy(worker);
    delete ui;
}

bool videoPage::init()
{
    QString vedio = GBK2QString("屏幕录制");
    QString camere = GBK2QString("摄像录制");
    QString network = GBK2QString("网络录制");
    ui->recordComboBox->addItem(QIcon(":/rc/video.svg"), vedio);
    ui->recordComboBox->addItem(QIcon(":/rc/camere.svg"), camere);
    ui->recordComboBox->addItem(QIcon(":/rc/network.svg"), network);

    ui->recordLabel->setObjectName("recordlabel");
    QString styleSheet =
            "QLabel#recordlabel {"
            "   background: none;          /* 清除继承的背景图片/颜色 */"
            "   background-color: #808080; /* 自定义灰色背景（覆盖父类蓝色） */"
            "   border: none;              /* 清除继承的边框 */"
            "   border: 1px solid #E0E0E0; /* 自定义边框 */"
            "   border-radius: 20px;       /* 圆角半径 */"
            "   color: white;              /* 文字白色，避免和黑色背景重叠 */"
            "   font: unset;               /* 重置字体为默认（如需自定义可替换） */"
            "   min-height: 30px;          /* 可选：保证Label有足够高度显示圆角 */"
            "}";
    ui->recordLabel->setStyleSheet(styleSheet);
    return true;
}

// 读取video目录视频并显示在tableWidget（列：视频名、时长、大小、日期、更多）
bool videoPage::initableWidget()
{
    // ========== 1. 定义视频目录路径（提前，确保优先处理目录） ==========
    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);

    // ========== 2. 强制创建目录（无论是否存在，确保目录一定有） ==========
    // 修复：用mkpath替代mkdir，支持多级目录创建；无需判断exists，直接创建
    if (!videoDir.mkpath(videoDirPath)) { // mkpath能创建多级目录（如a/b/c），mkdir仅创建一级
        QMessageBox::warning(this, GBK2QString("警告"),
                             GBK2QString("创建video目录失败！路径：") + videoDirPath);
        return false;
    }
    // 仅首次创建时提示（可选）
    static bool isDirCreatedTip = false;
    if (!isDirCreatedTip && !QDir(videoDirPath).exists()) {
        QMessageBox::information(this, GBK2QString("提示"),
                                 GBK2QString("video目录不存在，已自动创建：") + videoDirPath);
        isDirCreatedTip = true;
    }

    // ========== 3. 初始化TableWidget（核心：先设置表头/列数，再处理数据） ==========
    QStringList tableHeaders;
    tableHeaders << GBK2QString("视频名")
                 << GBK2QString("时长")
                 << GBK2QString("大小(MB)")
                 << GBK2QString("日期")
                 << GBK2QString("更多");

    // 修复1：无论是否有数据，先初始化列数和表头（确保表头一定显示）
    ui->tableWidget->setColumnCount(tableHeaders.size());
    ui->tableWidget->setHorizontalHeaderLabels(tableHeaders);

    // 表格样式优化（提前，确保无数据时样式也生效）
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true); // 最后一列自适应
    // 固定列宽（提前设置）
    ui->tableWidget->setColumnWidth(0, 200); // 视频名
    ui->tableWidget->setColumnWidth(1, 100); // 时长
    ui->tableWidget->setColumnWidth(2, 100); // 大小
    ui->tableWidget->setColumnWidth(3, 180); // 日期

    // ========== 4. 遍历视频文件 ==========
    QStringList videoFilters;
    videoFilters << "*.mp4" << "*.avi" << "*.mkv" << "*.flv" << "*.mov" << "*.wmv";
    QFileInfoList videoFileList = videoDir.entryInfoList(videoFilters,
                                                        QDir::Files | QDir::NoDotAndDotDot,
                                                        QDir::Name);

    // ========== 5. 有数据时填充表格 ==========
    if (!videoFileList.isEmpty()) {
        ui->tableWidget->setRowCount(videoFileList.size());
        for (int i = 0; i < videoFileList.size(); ++i) {
            QFileInfo fileInfo = videoFileList.at(i);
            QString videoPath = fileInfo.absoluteFilePath();

            // 5.1 视频名（不含后缀）
            QTableWidgetItem *nameItem = new QTableWidgetItem(fileInfo.baseName());
            nameItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 0, nameItem);

            // 5.2 时长
            double duration = AvWorker_getDuration(worker,videoPath.toStdString().c_str());
            //QString duration = getVideoDuration(videoPath);
            QTableWidgetItem *durationItem = new QTableWidgetItem(duration);
            durationItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 1, durationItem);

            // 5.3 大小（MB，保留2位小数）
            double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(QString::asprintf("%.2f", fileSizeMB));
            sizeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 2, sizeItem);

            // 5.4 日期
            QDateTime createTime = fileInfo.birthTime().isNull() ? fileInfo.lastModified() : fileInfo.birthTime();
            QString timeStr = createTime.toString("yyyy-MM-dd HH:mm:ss");
            QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 3, timeItem);

            // 5.5 更多（操作按钮）
            QWidget *btnWidget = new QWidget();
            QPushButton *openBtn = new QPushButton(GBK2QString("打开目录"));
            QPushButton *delBtn = new QPushButton(GBK2QString("删除"));
            // 按钮样式
            openBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#409EFF; color:white;}");
            delBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#F56C6C; color:white;}");
            // 布局
            QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->addWidget(openBtn);
            btnLayout->addWidget(delBtn);
            btnLayout->setSpacing(5);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            btnWidget->setLayout(btnLayout);
            ui->tableWidget->setCellWidget(i, 4, btnWidget);

            // 绑定按钮事件
            connect(openBtn, &QPushButton::clicked, this, [=]() {
                QProcess::startDetached("explorer.exe", QStringList() << "/select," << videoPath);
            });
            connect(delBtn, &QPushButton::clicked, this, [=]() {
                int ret = QMessageBox::question(this, GBK2QString("确认"),
                                               GBK2QString("是否删除文件：") + fileInfo.fileName());
                if (ret == QMessageBox::Yes) {
                    QFile::remove(videoPath);
                    ui->tableWidget->removeRow(i);
                }
            });
        }
    }
    // ========== 6. 无数据时的处理（修复核心：确保表头已初始化） ==========
    else {
        // 修复2：先清空表格，再设置空行，确保列数是5列
        ui->tableWidget->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem(GBK2QString("当前video目录下无视频文件"));
        emptyItem->setForeground(Qt::gray);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, emptyItem);
        // 修复3：跨列数必须和tableHeaders.size()一致（5列）
        ui->tableWidget->setSpan(0, 0, 1, tableHeaders.size());
    }

    return true;
}

