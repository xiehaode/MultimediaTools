#include <src/utils/encodinghelper.h>
#include "videopage.h"
#include "ui_videopage.h"
#include "src/utils/myipcmgr.h"
#include "src/base/mplayermanager.h"
#include <QString>

#include <QStyle>
#include <qdebug.h>
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
#include <QFileDialog>


videoPage::videoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::videoPage)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //mainLayout->addWidget(ui->layoutWidget);
    this->setLayout(mainLayout);

    init();

    worker = AvWorker_Create();
    if (!worker) {
        qDebug() << "AvWorker_Create failed!";
    }
    initableWidget();
}


videoPage::~videoPage()
{
    AvWorker_Destroy(worker);
    delete ui;
}

bool videoPage::init()
{
    QString vedio = GBK2QString("录制视频");
    QString camere = GBK2QString("摄像头");
    QString network = GBK2QString("录屏");
    ui->recordComboBox->addItem(QIcon(":/rc/video.svg"), vedio);
    ui->recordComboBox->addItem(QIcon(":/rc/camere.svg"), camere);
    ui->recordComboBox->addItem(QIcon(":/rc/network.svg"), network);

    ui->recordLabel->setObjectName("recordlabel");
    QString styleSheet =
            "QLabel#recordlabel {"
            "   background: none;          "
            "   background-color: #808080; "
            "   border: none;              "
            "   border: 1px solid #E0E0E0; "
            "   border-radius: 20px;       "
            "   color: white;              "
            "   font: unset;               "
            "   min-height: 30px;          "
            "}";
    ui->recordLabel->setStyleSheet(styleSheet);
    return true;
}


bool videoPage::initableWidget()
{
    // 初始化视频存储目录
    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);

    // 创建视频目录（如果不存在）
    if (!videoDir.mkpath(videoDirPath)) {
        QMessageBox::warning(this, GBK2QString("警告"),
                             GBK2QString("创建video目录失败：") + videoDirPath);
        return false;
    }

    // 目录创建提示（仅首次显示）
    static bool isDirCreatedTip = false;
    if (!isDirCreatedTip && !QDir(videoDirPath).exists()) {
        QMessageBox::information(this, GBK2QString("提示"),
                                 GBK2QString("已创建video目录：") + videoDirPath);
        isDirCreatedTip = true;
    }

    // 设置表格表头
    QStringList tableHeaders;
    tableHeaders << GBK2QString("视频名称")
                 << GBK2QString("时长")
                 << GBK2QString("大小(MB)")
                 << GBK2QString("创建时间")
                 << GBK2QString("操作");

    ui->tableWidget->setColumnCount(tableHeaders.size());
    ui->tableWidget->setHorizontalHeaderLabels(tableHeaders);

    // 设置表格属性
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 180);

    // 获取视频目录下的所有视频文件
    QStringList videoFilters;
    videoFilters << "*.mp4" << "*.avi" << "*.mkv" << "*.flv" << "*.mov" << "*.wmv";
    QFileInfoList videoFileList = videoDir.entryInfoList(videoFilters,
                                                        QDir::Files | QDir::NoDotAndDotDot,
                                                        QDir::Name);

    // 填充表格数据
    if (!videoFileList.isEmpty()) {
        ui->tableWidget->setRowCount(videoFileList.size());
        for (int i = 0; i < videoFileList.size(); ++i) {
            QFileInfo fileInfo = videoFileList.at(i);
            QString videoPath = fileInfo.absoluteFilePath();

            // 生成视频首帧缩略图（临时BMP文件）
            QString bmpPath = QDir::tempPath() + "/" + fileInfo.baseName() + "_frame.bmp";
            QByteArray gbkVideoPath = QString2GBK(videoPath);
            QByteArray gbkBmpPath = QString2GBK(bmpPath);

            bool getFrameOk = AvWorker_GetVideoFirstFrame(
                worker,
                gbkVideoPath.constData(),
                gbkBmpPath.constData(),
                false
            );

            // 创建视频名称单元格（包含缩略图+名称）
            QWidget* nameWidget = new QWidget();
            QHBoxLayout* nameLayout = new QHBoxLayout(nameWidget);
            nameLayout->setContentsMargins(5, 2, 5, 2); // 设置内边距
            nameLayout->setSpacing(8);                  // 设置控件间距

            // 缩略图标签
            QLabel* imgLabel = new QLabel();
            imgLabel->setFixedSize(60, 40);
            imgLabel->setScaledContents(true);
            if (getFrameOk && QFile::exists(bmpPath)) {
                imgLabel->setPixmap(QPixmap(bmpPath).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                QFile::remove(bmpPath); // 删除临时文件
            } else {
                // 加载默认视频图标
                imgLabel->setPixmap(QPixmap(":/rc/video.svg").scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            // 视频名称标签
            QLabel* textLabel = new QLabel(fileInfo.baseName());
            textLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter); // 居中对齐
            textLabel->setStyleSheet("color: #333; font-size: 12px;");   // 设置样式

            // 添加控件到布局
            nameLayout->addWidget(imgLabel);
            nameLayout->addWidget(textLabel);
            nameLayout->addStretch(); // 拉伸空白区域

            ui->tableWidget->setCellWidget(i, 0, nameWidget);

            // 设置行高
            ui->tableWidget->setRowHeight(i, 45);

            qDebug() << "videoPath" << videoPath << " | " << QFile::exists(videoPath);

            // 获取视频时长
            QByteArray gbkPath = QString2GBK(videoPath);
            double duration = AvWorker_getDuration(worker, gbkPath.constData());

            // 格式化时长（例如：3.716秒 → "0:04"，21.27秒 → "0:21"）
            int minutes = static_cast<int>(duration) / 60;
            int seconds = static_cast<int>(duration) % 60;
            QString durationStr = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));

            // 填充时长列
            QTableWidgetItem *durationItem = new QTableWidgetItem(durationStr);
            durationItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 1, durationItem);
            qDebug()<<"duration:"<<duration;

            // 计算并填充文件大小（MB）
            double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(QString::asprintf("%.2f", fileSizeMB));
            sizeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 2, sizeItem);

            // 获取文件创建时间（优先创建时间，其次修改时间，最后当前时间）
            QDateTime fileTime;
            if (!fileInfo.birthTime().isNull()) {
                fileTime = fileInfo.birthTime(); // 创建时间
            } else if (!fileInfo.lastModified().isNull()) {
                fileTime = fileInfo.lastModified(); // 修改时间
            } else {
                fileTime = QDateTime::currentDateTime(); // 当前时间
            }

            // 格式化时间并填充
            QString timeStr = fileTime.toString("yyyy-MM-dd HH:mm:ss");
            QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 3, timeItem);

            // 创建操作按钮（打开+删除）
            QWidget *btnWidget = new QWidget();
            QPushButton *openBtn = new QPushButton(GBK2QString("打开"));
            QPushButton *delBtn = new QPushButton(GBK2QString("删除"));

            // 设置按钮样式
            openBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#409EFF; color:white;}");
            delBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#F56C6C; color:white;}");

            // 按钮布局
            QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->addWidget(openBtn);
            btnLayout->addWidget(delBtn);
            btnLayout->setSpacing(5);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            btnWidget->setLayout(btnLayout);
            ui->tableWidget->setCellWidget(i, 4, btnWidget);

            // 绑定打开按钮点击事件
            connect(openBtn, &QPushButton::clicked, this, [this, videoPath]() {
                if (!m_ipcMgr) {
                    // 无IPC管理器时，直接打开文件所在目录
                    QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(videoPath));
                    return;
                }

                // 获取播放器路径
                QString mplayerPath = QDir::currentPath();
                if (!QFile::exists(mplayerPath)) {
                    QMessageBox::warning(this, GBK2QString("错误"), GBK2QString("未找到mplayer播放器：") + mplayerPath);
                    return;
                }

                // 启动播放器进程
                m_ipcMgr->startChildProcess(mplayerPath, false);

                // 发送播放视频指令并激活窗口
                m_ipcMgr->sendMessage("play_video:" + videoPath);
                m_ipcMgr->activateWindow();
            });

            // 绑定删除按钮点击事件
            connect(delBtn, &QPushButton::clicked, this, [this, videoPath, fileInfo]() {
                int ret = QMessageBox::question(this, GBK2QString("确认删除"),
                                               GBK2QString("确定要删除该文件吗？\n") + fileInfo.fileName());
                if (ret == QMessageBox::Yes) {
                    if (QFile::remove(videoPath)) {
                        // 删除成功后重新初始化表格
                        initableWidget();
                    } else {
                        QMessageBox::critical(this, GBK2QString("错误"), GBK2QString("文件删除失败，可能被占用。"));
                    }
                }
            });
        }
    } else {
        // 无视频文件时显示空提示
        ui->tableWidget->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem(GBK2QString("暂无视频文件"));
        emptyItem->setForeground(Qt::gray);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, emptyItem);
        ui->tableWidget->setSpan(0, 0, 1, tableHeaders.size());
    }

    return true;
}


void videoPage::on_begin_clicked()
{
    // 获取播放器路径并启动进程
    QString mplayerPath = QCoreApplication::applicationDirPath() + "/mPlayer.exe";
    process = new QProcess(this);
    process->start(mplayerPath);

    if (process->waitForStarted()) {
        // 启动成功后禁用开始按钮
        ui->begin->setDisabled(true);
    } else {
        // 启动失败提示
        QMessageBox::critical(NULL, "错误", "启动播放器失败: " + process->errorString(), QMessageBox::Yes);
    }

    // 进程结束后恢复按钮状态
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=]() {
        ui->begin->setDisabled(false);
    });
}

void videoPage::on_import_2_clicked()
{
    // 确保视频目录存在
    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);

    if (!videoDir.mkpath(videoDirPath)) {
        QMessageBox::warning(this, GBK2QString("选择要导入的视频"),
                             GBK2QString("创建video目录失败"));
        return;
    }

    // 打开文件选择对话框
    QString videoFilter = GBK2QString("视频文件 (*.mp4 *.avi *.mkv *.flv *.mov *.wmv);;所有文件 (*.*)");
    QString selectedFilePath = QFileDialog::getOpenFileName(
                this,
                GBK2QString("选择要导入的文件"),
                QDir::homePath(), // 默认打开用户主目录
                videoFilter
                );

    // 用户取消选择则返回
    if (selectedFilePath.isEmpty()) {
        return;
    }

    // 构造目标文件路径（避免重名）
    QFileInfo srcFileInfo(selectedFilePath);
    QString dstFileName = srcFileInfo.fileName();
    QString dstFilePath = videoDirPath + "/" + dstFileName;

    // 处理文件名重复（自动加序号）
    int fileIndex = 1;
    QString baseName = srcFileInfo.baseName();
    QString suffix = srcFileInfo.suffix();
    while (QFile::exists(dstFilePath)) {
        // 示例：test.mp4 → test(1).mp4
        dstFileName = QString("%1(%2).%3").arg(baseName).arg(fileIndex++).arg(suffix);
        dstFilePath = videoDirPath + "/" + dstFileName;
    }

    // 复制文件到视频目录
    QFile srcFile(selectedFilePath);
    bool copyOk = srcFile.copy(dstFilePath);
    if (!copyOk) {
        QMessageBox::critical(this, GBK2QString("错误"),
                              GBK2QString("文件导入失败：") + srcFile.errorString());
        return;
    }

    // 导入成功提示
    QMessageBox::information(this, GBK2QString("成功"),
                             GBK2QString("文件已导入：") + dstFilePath);

    // 重新初始化表格显示新文件
    initableWidget();
}
