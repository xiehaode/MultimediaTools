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
    bool select_Mode(Mode m);
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
