#ifndef MPLAYER_H
#define MPLAYER_H

#include "mglwidget.h"
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
class mpalyer : public QWidget
{
    Q_OBJECT
public:
    explicit mpalyer(QWidget *parent = nullptr);
    bool controlInit();
private:
    mGLWidget window;
    player p;
    QPushButton * play=NULL;
    QPushButton* stop=NULL;
    QPushButton* pause=NULL;
    QProgressBar *videoProcessBar=NULL;
signals:

};

#endif // MPLAYER_H
