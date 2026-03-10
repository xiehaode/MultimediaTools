#ifndef SMOOTH_H
#define SMOOTH_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class smooth;
}

class smooth : public QWidget
{
    Q_OBJECT

public:
    explicit smooth(QWidget *parent = nullptr);
    ~smooth();

    param getParams() const;

private:
    Ui::smooth *ui;
};

#endif // SMOOTH_H
