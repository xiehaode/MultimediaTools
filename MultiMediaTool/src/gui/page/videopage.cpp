#include <src/utils/encodinghelper.h>
#include "videopage.h"
#include "ui_videopage.h"
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
    init();
    worker = AvWorker_Create();
    initableWidget();

}

videoPage::~videoPage()
{
    AvWorker_Destroy(worker);
    delete ui;
}

bool videoPage::init()
{
    QString vedio = GBK2QString("");
    QString camere = GBK2QString("");
    QString network = GBK2QString("");
    ui->recordComboBox->addItem(QIcon(":/rc/video.svg"), vedio);
    ui->recordComboBox->addItem(QIcon(":/rc/camere.svg"), camere);
    ui->recordComboBox->addItem(QIcon(":/rc/network.svg"), network);

    ui->recordLabel->setObjectName("recordlabel");
    QString styleSheet =
            "QLabel#recordlabel {"
            "   background: none;          /* е/ */"
            "   background-color: #808080; /*  */"
            "   border: none;              /* е */"
            "   border: 1px solid #E0E0E0; /*  */"
            "   border-radius: 20px;       /*  */"
            "   color: white;              /*  */"
            "   font: unset;               /* 滻 */"
            "   min-height: 30px;          /* Label */"
            "}";
    ui->recordLabel->setStyleSheet(styleSheet);
    return true;
}


bool videoPage::initableWidget()
{

    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);



    if (!videoDir.mkpath(videoDirPath)) { // mkpatha/b/cmkdir
        QMessageBox::warning(this, GBK2QString(""),
                             GBK2QString("video·") + videoDirPath);
        return false;
    }

    static bool isDirCreatedTip = false;
    if (!isDirCreatedTip && !QDir(videoDirPath).exists()) {
        QMessageBox::information(this, GBK2QString(""),
                                 GBK2QString("video") + videoDirPath);
        isDirCreatedTip = true;
    }


    QStringList tableHeaders;
    tableHeaders << GBK2QString("视频名")
                 << GBK2QString("时长")
                 << GBK2QString("大小(MB)")
                 << GBK2QString("传建/当前时间")
                 << GBK2QString("编辑");


    ui->tableWidget->setColumnCount(tableHeaders.size());
    ui->tableWidget->setHorizontalHeaderLabels(tableHeaders);


    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true); // 

    ui->tableWidget->setColumnWidth(0, 200); // 
    ui->tableWidget->setColumnWidth(1, 100); // 
    ui->tableWidget->setColumnWidth(2, 100); // С
    ui->tableWidget->setColumnWidth(3, 180); // 


    QStringList videoFilters;
    videoFilters << "*.mp4" << "*.avi" << "*.mkv" << "*.flv" << "*.mov" << "*.wmv";
    QFileInfoList videoFileList = videoDir.entryInfoList(videoFilters,
                                                        QDir::Files | QDir::NoDotAndDotDot,
                                                        QDir::Name);


    if (!videoFileList.isEmpty()) {
        ui->tableWidget->setRowCount(videoFileList.size());
        for (int i = 0; i < videoFileList.size(); ++i) {
            QFileInfo fileInfo = videoFileList.at(i);
            QString videoPath = fileInfo.absoluteFilePath();

//
//            QTableWidgetItem *nameItem = new QTableWidgetItem(fileInfo.baseName());
//            nameItem->setTextAlignment(Qt::AlignCenter);
//            ui->tableWidget->setItem(i, 0, nameItem);

//            AvWorker_GetVideoFirstFrame(worker,"1.mp4","1.bmp",false);

            QString bmpPath = QDir::tempPath() + "/" + fileInfo.baseName() + "_frame.bmp"; // BMP·
            bool getFrameOk = AvWorker_GetVideoFirstFrame(
                worker,
                QString2GBK(videoPath).toStdString().c_str(),  // ·GBK
                QString2GBK(bmpPath).toStdString().c_str(),    // BMP·GBK
                false                            // RTSP
            );


            QWidget* nameWidget = new QWidget();
            QHBoxLayout* nameLayout = new QHBoxLayout(nameWidget);
            nameLayout->setContentsMargins(5, 2, 5, 2); // С
            nameLayout->setSpacing(8);                  // 


            QLabel* imgLabel = new QLabel();
            imgLabel->setFixedSize(60, 40); // Си
            imgLabel->setScaledContents(true); // LabelС
            if (getFrameOk && QFile::exists(bmpPath)) {
                imgLabel->setPixmap(QPixmap(bmpPath).scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

                QFile::remove(bmpPath);
            } else {
                // 
                imgLabel->setPixmap(QPixmap(":/rc/video.svg").scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }


            QLabel* textLabel = new QLabel(fileInfo.baseName());
            textLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter); // 
            textLabel->setStyleSheet("color: #333; font-size: 12px;");   // 


            nameLayout->addWidget(imgLabel);
            nameLayout->addWidget(textLabel);
            nameLayout->addStretch(); // 


            ui->tableWidget->setCellWidget(i, 0, nameWidget);


            ui->tableWidget->setRowHeight(i, 45);



            qDebug() << "·" << videoPath << " | " << QFile::exists(videoPath);
            double duration = AvWorker_getDuration(worker,QString2GBK(videoPath).toStdString().c_str());

            ///  :  3.716   "0:04"21.27   "0:21"
            int minutes = static_cast<int>(duration) / 60;
            int seconds = static_cast<int>(duration) % 60;
            QString durationStr = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));


            QTableWidgetItem *durationItem = new QTableWidgetItem(durationStr);
            durationItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 1, durationItem);
            qDebug()<<"duration:"<<duration;



            double fileSizeMB = fileInfo.size() / (1024.0 * 1024.0);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(QString::asprintf("%.2f", fileSizeMB));
            sizeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 2, sizeItem);


            //QDateTime createTime = fileInfo.birthTime().isNull() ? fileInfo.lastModified() : fileInfo.birthTime();
            QDateTime fileTime;
            if (!fileInfo.birthTime().isNull()) {
                fileTime = fileInfo.birthTime(); // 
            } else if (!fileInfo.lastModified().isNull()) {
                fileTime = fileInfo.lastModified(); // 
            } else {
                fileTime = QDateTime::currentDateTime(); // 
            }

            //QString timeStr = createTime.toString("yyyy-MM-dd HH:mm:ss");
            QString timeStr = fileTime.toString("yyyy-MM-dd HH:mm:ss");
            QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(i, 3, timeItem);


            QWidget *btnWidget = new QWidget();
            QPushButton *openBtn = new QPushButton(GBK2QString("打开"));
            QPushButton *delBtn = new QPushButton(GBK2QString("删除"));
            // 
            openBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#409EFF; color:white;}");
            delBtn->setStyleSheet("QPushButton{padding:2px 8px; border-radius:4px; background:#F56C6C; color:white;}");
            // 
            QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->addWidget(openBtn);
            btnLayout->addWidget(delBtn);
            btnLayout->setSpacing(5);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            btnWidget->setLayout(btnLayout);
            ui->tableWidget->setCellWidget(i, 4, btnWidget);

            // 
            connect(openBtn, &QPushButton::clicked, this, [=]() {
                QProcess::startDetached("explorer.exe", QStringList() << "/select," << videoPath);
            });
            connect(delBtn, &QPushButton::clicked, this, [=]() {
                int ret = QMessageBox::question(this, GBK2QString(""),
                                               GBK2QString("") + fileInfo.fileName());
                if (ret == QMessageBox::Yes) {
                    QFile::remove(videoPath);
                    ui->tableWidget->removeRow(i);
                }
            });
        }
    }
    else {

        ui->tableWidget->setRowCount(1);
        QTableWidgetItem *emptyItem = new QTableWidgetItem(GBK2QString("video"));
        emptyItem->setForeground(Qt::gray);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, 0, emptyItem);

        ui->tableWidget->setSpan(0, 0, 1, tableHeaders.size());
    }

    return true;
}


void videoPage::on_begin_clicked()
{

    process = new QProcess;
    process->start("mPlayer.exe");
    if(process->isOpen()){
        ui->begin->setDisabled(true);
    }
    else{
        QMessageBox::critical(NULL, "critical", "打开子进程失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
    // 一次性连接finished信号，进程结束后统一处理（无需在on_begin_clicked中重复连接）
    // 推荐的连接方式 (Qt 5 以后的函数指针语法)
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](){
        ui->begin->setDisabled(false);
    });
}




void videoPage::on_import_2_clicked()
{

    QString videoDirPath = QDir::currentPath() + "/video";
    QDir videoDir(videoDirPath);


    if (!videoDir.mkpath(videoDirPath)) {
        QMessageBox::warning(this, GBK2QString(""),
                             GBK2QString("video"));
        return;
    }


    QString videoFilter = GBK2QString(" (*.mp4 *.avi *.mkv *.flv *.mov *.wmv);; (*.*)");
    QString selectedFilePath = QFileDialog::getOpenFileName(
                this,
                GBK2QString(""),
                QDir::homePath(), // 
                videoFilter
                );


    if (selectedFilePath.isEmpty()) {
        return;
    }


    QFileInfo srcFileInfo(selectedFilePath);
    QString dstFileName = srcFileInfo.fileName(); // 
    QString dstFilePath = videoDirPath + "/" + dstFileName;


    int fileIndex = 1;
    QString baseName = srcFileInfo.baseName();
    QString suffix = srcFileInfo.suffix();
    while (QFile::exists(dstFilePath)) {
        // 磺test.mp4  test(1).mp4
        dstFileName = QString("%1(%2).%3").arg(baseName).arg(fileIndex++).arg(suffix);
        dstFilePath = videoDirPath + "/" + dstFileName;
    }


    QFile srcFile(selectedFilePath);
    bool copyOk = srcFile.copy(dstFilePath);
    if (!copyOk) {
        QMessageBox::critical(this, GBK2QString(""),
                              GBK2QString("") + srcFile.errorString());
        return;
    }


    QMessageBox::information(this, GBK2QString(""),
                             GBK2QString("") + dstFilePath);


    initableWidget();
}
