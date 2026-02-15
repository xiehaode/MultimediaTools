#include <src/utils/encodinghelper.h>
#include "videopage.h"
#include "ui_videopage.h"
#include "src/utils/myipcmgr.h"
#include "src/base/mplayermanager.h"

#include <QString>
#include <QWidget>
#include <QVBoxLayout>
#include <QStyle>
#include <QDebug>
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
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>

// 声明GBK转UTF8函数（确保你的项目中有这个函数的实现）
std::string gbk_to_utf8(const std::string& gbk_str);

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
    QString vedio = QString::fromUtf8(gbk_to_utf8("录制视频").c_str());
    QString camere = QString::fromUtf8(gbk_to_utf8("摄像头").c_str());
    QString network = QString::fromUtf8(gbk_to_utf8("录制").c_str());
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
        QMessageBox::warning(this, QString::fromUtf8(gbk_to_utf8("错误").c_str()),
                             QString::fromUtf8(gbk_to_utf8("创建video目录失败：").c_str()) + videoDirPath);
        return false;
    }

    // 目录创建成功提示（只显示一次）
    static bool isDirCreatedTip = false;
    if (!isDirCreatedTip && !QDir(videoDirPath).exists()) {
        QMessageBox::information(this, QString::fromUtf8(gbk_to_utf8("提示").c_str()),
                                 QString::fromUtf8(gbk_to_utf8("已创建video目录：").c_str()) + videoDirPath);
        isDirCreatedTip = true;
    }

    // 设置表格表头
    QStringList tableHeaders;
    tableHeaders << QString::fromUtf8(gbk_to_utf8("视频名称").c_str())
                 << QString::fromUtf8(gbk_to_utf8("时长").c_str())
                 << QString::fromUtf8(gbk_to_utf8("大小(MB)").c_str())
                 << QString::fromUtf8(gbk_to_utf8("创建时间").c_str())
                 << QString::fromUtf8(gbk_to_utf8("操作").c_str());

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

    // 填充表格
    if (!videoFileList.isEmpty()) {
        ui->tableWidget->setRowCount(videoFileList.size());
        for (int i = 0; i < videoFileList.size(); ++i) {
            QFileInfo fileInfo = videoFileList.at(i);
            QString videoPath = fileInfo.absoluteFilePath();

            // 获取视频首帧作为预览图（临时BMP文件）
            QString bmpPath = QDir::tempPath() + "/" + fileInfo.baseName() + "_frame.bmp";
            QByteArray gbkVideoPath = QString2GBK(videoPath);
            QByteArray gbkBmpPath = QString2GBK(bmpPath);

            bool getFrameOk = AvWorker_GetVideoFirstFrame(
                worker,
                gbkVideoPath.constData(),
                gbkBmpPath.constData(),
                false
            );

            // 视频名称单元格（预览图+名称）
            QWidget* nameWidget = new QWidget();
            QHBoxLayout* nameLayout = new QHBoxLayout(nameWidget);
            nameLayout->setContentsMargins(5, 2, 5, 2); // 设置内边距
            nameLayout->setSpacing(8);                  // 设置控件间距

            // 预览图标签
            QLabel* imgLabel = new QLabel();
            imgLabel->setFixedSize(60, 40);
            imgLabel->setScaledContents(true);
            if (getFrameOk && QFile::exists(bmpPath)) {
                imgLabel->setPixmap(QPixmap(bmpPath).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                QFile::remove(bmpPath); // 删除临时文件
            } else {
                // 设置默认视频图标
                imgLabel->setPixmap(QPixmap(":/rc/video.svg").scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            // 视频名称标签
            QLabel* textLabel = new QLabel(fileInfo.baseName());
            textLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter); // 居中对齐
            textLabel->setStyleSheet("color: #333; font-size: 12px;");   // 设置样式

            // 添加控件到布局
            nameLayout->addWidget(imgLabel);
            nameLayout->addWidget(textLabel);
            nameLayout->addStretch(); // 填充空白区域

            ui->tableWidget->setCellWidget(i, 0, nameWidget);

            // 设置行高
            ui->tableWidget->setRowHeight(i, 45);

            qDebug() << "videoPath" << videoPath << " | " << QFile::exists(videoPath);

            // 获取视频时长
            QByteArray gbkPath = QString2GBK(videoPath);
            double duration = AvWorker_getDuration(worker, gbkPath.constData());

            // 格式化时长（例如：3.716秒 -> "0:04", 21.27秒 -> "0:21"）
            int minutes = static_cast<int>(duration) / 60;
            int seconds = static_cast<int>(duration) % 60;
            QString durationStr = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));

            // 设置时长
            QTableWidgetItem *durationItem = new QTableWidgetItem(durationStr);
            durationItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 1, durationItem);
            qDebug()<<"duration:"<<duration;

            // 计算并设置文件大小（MB）
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

            // 格式化时间并设置
            QString timeStr = fileTime.toString("yyyy-MM-dd HH:mm:ss");
            QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 3, timeItem);

            // 设置操作按钮（打开+删除）
            QWidget *btnWidget = new QWidget();
            QPushButton *openBtn = new QPushButton(QString::fromUtf8(gbk_to_utf8("打开").c_str()));
            QPushButton *delBtn = new QPushButton(QString::fromUtf8(gbk_to_utf8("删除").c_str()));

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
                    // 无IPC管理时，直接打开文件所在目录
                    QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(videoPath));
                    return;
                }

                // 查找播放器路径
                QString mplayerPath;
                QStringList possiblePaths;
                
                // 获取应用程序目录
                QString appDir = QCoreApplication::applicationDirPath();
                
                // 添加可能的路径
                possiblePaths << appDir + "/mPlayer.exe";                    // 应用程序目录
                possiblePaths << appDir + "/../bin/mPlayer.exe";             // 开发环境的bin目录
                possiblePaths << QDir::currentPath() + "/mPlayer.exe";       // 当前工作目录
                possiblePaths << QDir::currentPath() + "/bin/mPlayer.exe";    // 当前目录的bin子目录
                
                // 查找第一个存在的路径
                for (const QString& path : possiblePaths) {
                    if (QFileInfo::exists(path)) {
                        mplayerPath = path;
                        break;
                    }
                }
                
                // 如果没找到播放器，直接用系统默认播放器打开
                if (mplayerPath.isEmpty()) {
                    qDebug() << "[VideoPage] 未找到mPlayer，使用系统默认播放器打开视频:" << videoPath;
                    QDesktopServices::openUrl(QUrl::fromLocalFile(videoPath));
                    return;
                }

                qDebug() << "[VideoPage] 使用mPlayer打开视频:" << videoPath;
                
                // 启动播放器进程
                m_ipcMgr->startChildProcess(mplayerPath, false);

                // 发送播放视频指令并激活窗口
                m_ipcMgr->sendMessage("play_video:" + videoPath);
                m_ipcMgr->activateWindow();
            });

            // 绑定删除按钮点击事件
            connect(delBtn, &QPushButton::clicked, this, [this, videoPath, fileInfo]() {
                int ret = QMessageBox::question(this, QString::fromUtf8(gbk_to_utf8("确认删除").c_str()),
                                               QString::fromUtf8(gbk_to_utf8("确定要删除该文件吗？\n").c_str()) + fileInfo.fileName());
                if (ret == QMessageBox::Yes) {
                    if (QFile::remove(videoPath)) {
                        // 删除成功后重新初始化表格
                        initableWidget();
                    } else {
                        QMessageBox::critical(this, QString::fromUtf8(gbk_to_utf8("错误").c_str()),
                                             QString::fromUtf8(gbk_to_utf8("文件删除失败，可能被占用。").c_str()));
                    }
                }
            });
        }
    } else {
        // 无视频文件时显示提示信息
        ui->tableWidget->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem(QString::fromUtf8(gbk_to_utf8("暂无视频文件").c_str()));
        emptyItem->setForeground(Qt::gray);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, emptyItem);
        ui->tableWidget->setSpan(0, 0, 1, tableHeaders.size());
    }

    return true;
}


void videoPage::on_begin_clicked()
{
    // 尝试多个可能的播放器路径
    QString mplayerPath;
    QStringList possiblePaths;
    
    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    
    qDebug() << "[VideoPage] 开始查找mPlayer路径";
    qDebug() << "[VideoPage] 应用程序目录:" << appDir;
    qDebug() << "[VideoPage] 当前工作目录:" << QDir::currentPath();
    
    // 添加可能的路径
    possiblePaths << appDir + "/mPlayer.exe";                    // 应用程序目录
    possiblePaths << appDir + "/../bin/mPlayer.exe";             // 开发环境的bin目录
    possiblePaths << QDir::currentPath() + "/mPlayer.exe";       // 当前工作目录
    possiblePaths << QDir::currentPath() + "/bin/mPlayer.exe";    // 当前目录的bin子目录
    possiblePaths << "mPlayer.exe";                              // 系统PATH中查找
    
    // 查找第一个存在的路径
    for (const QString& path : possiblePaths) {
        qDebug() << "[VideoPage] 检查路径:" << path;
        if (QFileInfo::exists(path)) {
            mplayerPath = path;
            qDebug() << "[VideoPage] 找到mPlayer路径:" << mplayerPath;
            break;
        }
    }
    
    // 如果没找到，显示错误
    if (mplayerPath.isEmpty()) {
        QString errorMsg = QString::fromUtf8(gbk_to_utf8("未找到mPlayer播放器。请确保mPlayer.exe在以下位置之一：\n").c_str());
        errorMsg += possiblePaths.join("\n");
        QMessageBox::critical(this, QString::fromUtf8(gbk_to_utf8("错误").c_str()), errorMsg);
        qDebug() << "[VideoPage] 错误: 未找到mPlayer.exe";
        return;
    }
    
    qDebug() << "[VideoPage] 启动播放器:" << mplayerPath;
    process = new QProcess(this);
    
    // 设置工作目录为播放器所在目录
    QFileInfo mplayerInfo(mplayerPath);
    process->setWorkingDirectory(mplayerInfo.absolutePath());
    
    // 启动进程
    process->start(mplayerPath);
    
    // 使用超时机制，等待最多5秒
    if (process->waitForStarted(5000)) { 
        // 启动成功禁用开始按钮
        ui->begin->setDisabled(true);
        qDebug() << "[VideoPage] 播放器启动成功";
    } else {
        // 启动失败提示
        QString errorMsg = QString::fromUtf8(gbk_to_utf8("播放器启动失败").c_str()) + ": " + process->errorString();
        QMessageBox::critical(this, QString::fromUtf8(gbk_to_utf8("错误").c_str()), errorMsg);
        qDebug() << "[VideoPage] 播放器启动失败:" << process->errorString();
        qDebug() << "[VideoPage] 播放器路径:" << mplayerPath;
        delete process;
        process = nullptr;
        return;
    }

    // 绑定进程结束信号恢复按钮状态
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "[VideoPage] 播放器进程结束，退出码:" << exitCode << "状态:" << exitStatus;
        ui->begin->setDisabled(false);
        delete process;
        process = nullptr;
    });
    
    // 连接错误信号，提供更好的错误处理
    connect(process, &QProcess::errorOccurred, [=](QProcess::ProcessError error) {
        qDebug() << "[VideoPage] 播放器进程错误:" << error << process->errorString();
        ui->begin->setDisabled(false);
    });
    
    // 连接输出信号，用于调试
    connect(process, &QProcess::readyReadStandardOutput, [=]() {
        QByteArray output = process->readAllStandardOutput();
        qDebug() << "[VideoPage] 播放器输出:" << output;
    });
    
    connect(process, &QProcess::readyReadStandardError, [=]() {
        QByteArray error = process->readAllStandardError();
        qDebug() << "[VideoPage] 播放器错误输出:" << error;
    });
}

void videoPage::on_import_2_clicked()
{
    // 确保视频目录存在
    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);

    if (!videoDir.mkpath(videoDirPath)) {
        QMessageBox::warning(this, QString::fromUtf8(gbk_to_utf8("选择要导入的视频").c_str()),
                             QString::fromUtf8(gbk_to_utf8("创建video目录失败").c_str()));
        return;
    }

    // 文件选择对话框
    QString videoFilter = QString::fromUtf8(gbk_to_utf8("视频文件 (*.mp4 *.avi *.mkv *.flv *.mov *.wmv);;所有文件 (*.*)").c_str());
    QString selectedFilePath = QFileDialog::getOpenFileName(
                this,
                QString::fromUtf8(gbk_to_utf8("选择要导入的文件").c_str()),
                QDir::homePath(), // 默认打开用户主目录
                videoFilter
                );

    // 用户取消选择返回
    if (selectedFilePath.isEmpty()) {
        return;
    }

    // 构建目标文件路径（避免重名）
    QFileInfo srcFileInfo(selectedFilePath);
    QString dstFileName = srcFileInfo.fileName();
    QString dstFilePath = videoDirPath + "/" + dstFileName;

    // 文件重名时自动重命名
    int fileIndex = 1;
    QString baseName = srcFileInfo.baseName();
    QString suffix = srcFileInfo.suffix();
    while (QFile::exists(dstFilePath)) {
        // 示例：test.mp4 -> test(1).mp4
        dstFileName = QString("%1(%2).%3").arg(baseName).arg(fileIndex++).arg(suffix);
        dstFilePath = videoDirPath + "/" + dstFileName;
    }

    // 复制文件到视频目录
    QFile srcFile(selectedFilePath);
    bool copyOk = srcFile.copy(dstFilePath);
    if (!copyOk) {
        QMessageBox::critical(this, QString::fromUtf8(gbk_to_utf8("错误").c_str()),
                              QString::fromUtf8(gbk_to_utf8("文件复制失败：").c_str()) + srcFile.errorString());
        return;
    }

    // 复制成功提示
    QMessageBox::information(this, QString::fromUtf8(gbk_to_utf8("成功").c_str()),
                             QString::fromUtf8(gbk_to_utf8("文件已导入：").c_str()) + dstFilePath);

    // 重新初始化表格显示新文件
    initableWidget();
}

void videoPage::on_pushButton_clicked()
{
    mconcat = new concat;
    mconcat->show();
    mconcat->setAttribute(Qt::WA_DeleteOnClose,true);
}

