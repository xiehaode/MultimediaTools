#ifndef VIDEOPAGE_H
#define VIDEOPAGE_H

#include <QWidget>
#include <QString>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class videoPage;
}

class videoPage : public QWidget
{
    Q_OBJECT

public:
    explicit videoPage(QWidget *parent = nullptr);
    ~videoPage();
    bool init();
    bool initableWidget();

private:
    void *worker = NULL;
    Ui::videoPage *ui;
};

#endif // VIDEOPAGE_H
