#ifndef VIDEOPAGE_H
#define VIDEOPAGE_H

#include <QWidget>
#include <QString>
#include "OpenCVFFMpegTools.h"
#include "src/base/pagebase.h"

namespace Ui {
class videoPage;
}

class videoPage : public QWidget ,pageBase
{
    Q_OBJECT

public:
    explicit videoPage(QWidget *parent = nullptr);
    ~videoPage();
    bool init();
    bool initableWidget();

private slots:
    void on_begin_clicked();


    void on_import_2_clicked();

private:
    void *worker = NULL;
    Ui::videoPage *ui;
};

#endif // VIDEOPAGE_H
