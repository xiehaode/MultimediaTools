#include "mplayer.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QFileDialog>
#include "mglwidget.h"
#include "player.h"
#include "mdevice.h"
mpalyer::mpalyer(QWidget *parent) : QWidget(parent)
{
    bool ret = controlInit();
    if(ret ==false){
        qDebug()<<"initControl Fair";
    }

    // 设置glwidget播放
    window.setPlayer(&p);

    connect(play, &QPushButton::clicked, this, [=]() {
        p.pause(false);
    });

    connect(pause, &QPushButton::clicked, this, [=]() {
        p.pause(true);
    });

    connect(selectMode, &QPushButton::clicked, this, [=]() {
         Mode m = box->currentData().value<Mode>();
         select_Mode(m);
    });

    connect(&p, &player::positionChanged, this, [=](int64_t ms, int64_t total_ms) {
        if (total_ms > 0) {
            int val = (int)(ms * 100 / total_ms);
            videoProcessBar->setValue(val);
            // 更新进度条上的文字显示 (当前秒数 / 总秒数)
            videoProcessBar->setFormat(QString("%1s / %2s").arg(ms/1000).arg(total_ms/1000));
        }
    });




}

bool mpalyer::controlInit()
{
    // 初始化所有控件，设置基础文本/属性
    play = new QPushButton(tr("播放"), this);
    selectMode = new QPushButton(tr("选择模式"), this);
    pause = new QPushButton(tr("暂停"), this);
    box = new QComboBox(this);
    QVariant capture(CAPTURE);
    QVariant video(VIDEO);
    box->addItem("camera",capture);
    box->addItem("video",video);
    box->setCurrentIndex(0);

    videoProcessBar = new QProgressBar(this);

    // 进度条基础设置：范围0-100，初始值0，显示百分比
    videoProcessBar->setRange(0, 100);
    videoProcessBar->setValue(0);//
    videoProcessBar->setFormat("current process: %p");


    // 主布局：垂直布局（整个播放器界面）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15); // 整体边距：上下左右15px
    mainLayout->setSpacing(12); // 控件之间的垂直间距12px

    // 控制按钮布局：水平布局（播放/暂停/停止）
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10); // 按钮之间的水平间距10px
    // 添加按钮到水平布局
    btnLayout->addWidget(play);
    btnLayout->addWidget(pause);
    btnLayout->addWidget(selectMode);
    btnLayout->addWidget(box);
    // 按钮区左右添加弹性空间，让按钮居中显示
    btnLayout->addStretch();
    btnLayout->insertStretch(0);

    // 向主布局添加控件/子布局（按从上到下顺序）
    mainLayout->addWidget(&window, 5); // OpenGL窗口占5份空间（核心区，占比最大）
    mainLayout->addWidget(videoProcessBar); // 播放进度条
    mainLayout->addLayout(btnLayout);   // 控制按钮组

    // OpenGL窗口：缩放时优先拉伸（占比最大）
    window.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 按钮：固定宽高，不随窗口缩放变化
    play->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pause->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    selectMode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // 按钮设置固定大小（统一尺寸，美观）
    QSize btnSize(80, 36);
    play->setFixedSize(btnSize);
    pause->setFixedSize(btnSize);
    selectMode->setFixedSize(btnSize);

    // 拼接QSS样式字符串，支持换行/注释，便于维护
    QString qss = R"(
        /* ------------- 全局基础样式 ------------- */
        QWidget {
            font-family: "Microsoft YaHei"; /* 微软雅黑，适配中文 */
            font-size: 14px;                /* 全局字体大小 */
            background-color: #F5F5F5;      /* 整体背景色：浅灰色 */
        }

        /* ------------- 播放/暂停/停止按钮 ------------- */
        QPushButton {
            color: #FFFFFF;                 /* 文字白色 */
            background-color: #2E86AB;      /* 主色调：深蓝色 */
            border: none;                   /* 隐藏默认边框 */
            border-radius: 12px;             /* 圆角6px */
            font-weight: bold;              /* 文字加粗 */
        }
        QPushButton:hover {
            background-color: #4A90E2;      /* 鼠标悬浮：浅一点的蓝色 */
        }
        QPushButton:pressed {
            background-color: #1F618D;      /* 鼠标按下：深一点的蓝色 */
            padding-left: 2px;              /* 按下轻微偏移，模拟按压感 */
            padding-top: 2px;
        }
        QPushButton:disabled {
            background-color: #B0BEC5;      /* 禁用状态：灰色 */
            color: #EEEEEE;
        }

        /* ------------- 播放进度条 ------------- */
        QProgressBar {
            height: 8px;                    /* 进度条高度 */
            border-radius: 4px;             /* 圆角4px，与高度匹配 */
            background-color: #E0E0E0;      /* 进度条背景（未完成部分） */
            text-align: center;             /* 进度文字居中 */
            color: #666666;                 /* 进度文字颜色 */
            font-size: 12px;                /* 进度文字大小 */
        }
        QProgressBar::chunk {
            border-radius: 4px;             /* 进度块圆角，与整体一致 */
            background-color: #2E86AB;      /* 进度块颜色：与按钮主色调一致 */
        }

        /* ------------- 日志输出文本框 ------------- */
        QTextEdit {
            background-color: #FFFFFF;      /* 白色背景 */
            border: 1px solid #DDDDDD;      /* 浅灰色边框 */
            border-radius: 6px;             /* 圆角6px */
            padding: 8px;                   /* 内边距，避免文字贴边 */
            color: #333333;                 /* 文字颜色：深灰色 */
            font-size: 13px;                /* 日志文字稍小 */
        }
        QTextEdit:focus {
            border-color: #2E86AB;          /* 聚焦时边框变主色调，提示焦点 */
            outline: none;                  /* 隐藏默认聚焦外框 */
        }

        /* ------------- OpenGL视频渲染窗口 ------------- */
        mGLWidget {
            background-color: #000000;      /* 黑色背景，适配视频渲染（无视频时不突兀） */
            border: 2px solid #CCCCCC;      /* 浅灰色边框，区分视频区 */
            border-radius: 8px;             /* 圆角8px，提升美观 */
        }
    )";

    // 给当前播放器窗口设置QSS样式（所有子控件继承生效）
    this->setStyleSheet(qss);


    this->setLayout(mainLayout);
    window.show();
    // 初始化成功返回true
    return true;
}

bool mpalyer::select_Mode(Mode m)
{
    if(m ==CAPTURE){
        // 搜索摄像头并启动解码线程

        auto devices = getVideoDevices();
        if (!devices.empty()) {
            if (p.ffplayer_open(devices[0].name) == 0) {
            //if (p.ffplayer_open("D:/vsPro/Project5/Project5/2.mp4",false) == 0) {
                QThread* thread = new QThread();
                p.moveToThread(thread);

                QObject::connect(thread, &QThread::started, [=]() {
                    while (!p.isQuit()) {
                        p.ffplayer_read_frame();
                        QThread::msleep(10);
                    }
                });

                QObject::connect(&p, &player::frameReady, &window, &mGLWidget::onFrameReady, Qt::QueuedConnection);
                thread->start();
            }
        }
    }
    else if(m == VIDEO){
        QString file_name = QFileDialog::getOpenFileName(NULL, "标题", ".", "视频文件(*.mp4 *.avi *.mkv *.mov)");
        if (p.ffplayer_open(file_name,false) == 0) {
            //if (p.ffplayer_open("D:/vsPro/Project5/Project5/2.mp4",false) == 0) {
            QThread* thread = new QThread();
            p.moveToThread(thread);

            QObject::connect(thread, &QThread::started, [=]() {
                while (!p.isQuit()) {
                    p.ffplayer_read_frame();
                    QThread::msleep(10);
                }
            });

            QObject::connect(&p, &player::frameReady, &window, &mGLWidget::onFrameReady, Qt::QueuedConnection);
            thread->start();
            }

    }
    selectMode->setDisabled(true);
    return true;
}
