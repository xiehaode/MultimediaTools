#ifndef REVERSE_H
#define REVERSE_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class reverse;
}

class reverse : public QWidget
{
    Q_OBJECT

public:
    explicit reverse(QWidget *parent = nullptr);
    ~reverse();

    param getParams() const;

private:
    Ui::reverse *ui;
};

#endif // REVERSE_H
