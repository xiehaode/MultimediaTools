#ifndef MPLAYER_H
#define MPLAYER_H

#include "mglwidget.h"
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QMetaType>
#include <QSlider>
#include <QLabel>
class mpalyer : public QWidget
{

    Q_OBJECT
public:
    enum Mode{
        VIDEO,
        CAPTURE,
        SCREEN
    };


    explicit mpalyer(QWidget *parent = nullptr);
    bool controlInit();
    bool select_Mode(Mode m);
    void playVideo(const QString& path);
private:
    mGLWidget window;
    player p;
    QComboBox *box =NULL;
    QPushButton * play=NULL;
    QPushButton* selectMode=NULL;
    QPushButton* pause=NULL;
    QPushButton* record=NULL;
    QPushButton* pipBtn=NULL;
    QSlider *videoSlider=NULL;
    QLabel *timeLabel=NULL;
    bool m_isPipMode = false;
    bool m_isRecording = false;
    bool m_isUserSeeking = false;

signals:

};

Q_DECLARE_METATYPE(mpalyer::Mode)
#endif // MPLAYER_H
