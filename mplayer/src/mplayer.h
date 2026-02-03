#ifndef MPLAYER_H
#define MPLAYER_H

#include "mglwidget.h"
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QMetaType>
class mpalyer : public QWidget
{

    Q_OBJECT
public:
    enum Mode{
        VIDEO,
        CAPTURE
    };

    explicit mpalyer(QWidget *parent = nullptr);
    bool controlInit();
<<<<<<< HEAD
    bool select_Mode(Mode m);
=======
    void playVideo(const QString& path);
>>>>>>> 598de255f5feea8a06bc1b6944b20507c32379a5
private:
    mGLWidget window;
    player p;
    QComboBox *box =NULL;
    QPushButton * play=NULL;
    QPushButton* selectMode=NULL;
    QPushButton* pause=NULL;
    QProgressBar *videoProcessBar=NULL;
signals:

};
Q_DECLARE_METATYPE(mpalyer::Mode)
#endif // MPLAYER_H
