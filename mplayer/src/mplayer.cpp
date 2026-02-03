#include <QApplication>
#include <QDesktopWidget>
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

    connect(record, &QPushButton::clicked, this, [=]() {
        if (!p.isRecording()) {
            QString path = QFileDialog::getSaveFileName(this, tr("视频录制"), "recorded.mp4", tr("视频文件(*.mp4)"));
            if (path.isEmpty()) return;
            
            // 确保在录制前重置状态（防止旧的录制状态残留）
            if (p.startRecord(path) == 0) {
                record->setText(tr("停止录制"));
                record->setStyleSheet("background-color: #D32F2F; color: white;"); 
            } else {
                qDebug() << "Start record failed!";
            }
        } else {
            p.stopRecord();
            record->setText(tr("录制"));
            record->setStyleSheet(""); 
        }
    });


    connect(pipBtn, &QPushButton::clicked, this, [=]() {
        if (!m_isPipMode) {
            // 进入画中画模式
            window.setParent(nullptr); // 脱离主窗口
            window.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
            
            // 设置一个较小的尺寸并移动到右下角
            QRect screenRect = QApplication::desktop()->screenGeometry();
            int w = 320;
            int h = 180;
            window.setGeometry(screenRect.width() - w - 20, screenRect.height() - h - 50, w, h);
            window.show();
            
            pipBtn->setText(tr("退出画中画"));
            m_isPipMode = true;
        } else {
            // 退出画中画模式，还原到主布局
            window.setParent(this);
            window.setWindowFlags(Qt::Widget);
            
            // 重新插入布局
            QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
            if (mainLayout) {
                mainLayout->insertWidget(0, &window, 5);
            }
            window.show();
            
            pipBtn->setText(tr("画中画"));
            m_isPipMode = false;
        }
    });

    connect(selectMode, &QPushButton::clicked, this, [=]() {
         Mode m = box->currentData().value<Mode>();
         select_Mode(m);
    });

    connect(&p, &player::positionChanged, this, [=](long long ms, long long total_ms) {
        if (!m_isUserSeeking) {
            if (total_ms > 0) {
                videoSlider->setMaximum((int)total_ms);
                videoSlider->setValue((int)ms);
                
                int cur_sec = (int)(ms / 1000);
                int tot_sec = (int)(total_ms / 1000);
                QString curTime = QString("%1:%2").arg(cur_sec / 60, 2, 10, QChar('0')).arg(cur_sec % 60, 2, 10, QChar('0'));
                QString totalTime = QString("%1:%2").arg(tot_sec / 60, 2, 10, QChar('0')).arg(tot_sec % 60, 2, 10, QChar('0'));
                timeLabel->setText(curTime + " / " + totalTime);
            } else {
                int cur_sec = (int)(ms / 1000);
                QString curTime = QString("%1:%2").arg(cur_sec / 60, 2, 10, QChar('0')).arg(cur_sec % 60, 2, 10, QChar('0'));
                timeLabel->setText(curTime + " / LIVE");
                videoSlider->setMaximum(100);
                videoSlider->setValue(100);
            }
        }
    });



    connect(videoSlider, &QSlider::sliderPressed, this, [=]() {
        m_isUserSeeking = true;
    });

    connect(videoSlider, &QSlider::sliderReleased, this, [=]() {
        p.seek(videoSlider->value());
        m_isUserSeeking = false;
    });
}


void mpalyer::playVideo(const QString& path)
{
    qDebug() << "Playing video:" << path;
    p.stop();
    if (p.ffplayer_open(path.toStdString().c_str(), false) == 0) {
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
    box->addItem("screen", QVariant::fromValue(SCREEN));
    box->setCurrentIndex(0);


    record = new QPushButton(tr("录制"), this);
    pipBtn = new QPushButton(tr("画中画"), this);
    videoSlider = new QSlider(Qt::Horizontal, this);
    timeLabel = new QLabel("00:00 / 00:00", this);


    // 进度条基础设置
    videoSlider->setRange(0, 100);
    videoSlider->setValue(0);



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
    btnLayout->addWidget(record);
    btnLayout->addWidget(pipBtn);
    btnLayout->addWidget(selectMode);
    play->setDisabled(true);
    pause->setDisabled(true);
    record->setDisabled(true);
    pipBtn->setDisabled(true);

    btnLayout->addWidget(box);

    // 按钮区左右添加弹性空间，让按钮居中显示
    btnLayout->addStretch();
    btnLayout->insertStretch(0);

    // 向主布局添加控件/子布局（按从上到下顺序）
    mainLayout->addWidget(&window, 5); // OpenGL窗口占5份空间（核心区，占比最大）
    
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(videoSlider);
    sliderLayout->addWidget(timeLabel);
    mainLayout->addLayout(sliderLayout);
    
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
    record->setFixedSize(btnSize);
    pipBtn->setFixedSize(btnSize);
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
        QSlider::groove:horizontal {
            height: 6px;
            background: #E0E0E0;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #2E86AB;
            border: 1px solid #2E86AB;
            width: 14px;
            height: 14px;
            margin: -4px 0;
            border-radius: 7px;
        }
        QSlider::sub-page:horizontal {
            background: #2E86AB;
            border-radius: 3px;
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
    play->setDisabled(false);
    pause->setDisabled(false);
    record->setDisabled(false);
    pipBtn->setDisabled(false);
    if(m ==CAPTURE){
        // 搜索摄像头并启动解码线程
        play->setDisabled(true);
        pause->setDisabled(true);
        pipBtn->setDisabled(true);
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
    else if (m == SCREEN) { // SCREEN 模式
        play->setDisabled(true);
        pause->setDisabled(true);
        pipBtn->setDisabled(true);
        // FFmpeg 使用 gdigrab 录制 Windows 屏幕
        if (p.ffplayer_open("desktop", true) == 0) {
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
