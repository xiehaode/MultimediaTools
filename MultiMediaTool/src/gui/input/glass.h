#ifndef GLASS_H
#define GLASS_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class glass;
}

class glass : public QWidget
{
    Q_OBJECT

public:
    explicit glass(QWidget *parent = nullptr);
    ~glass();

    param getParams() const;

private:
    Ui::glass *ui;
};

#endif // GLASS_H
