#ifndef GREY_H
#define GREY_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class grey;
}

class grey : public QWidget
{
    Q_OBJECT

public:
    explicit grey(QWidget *parent = nullptr);
    ~grey();

    param getParams() const;

private:
    Ui::grey *ui;
};

#endif // GREY_H
