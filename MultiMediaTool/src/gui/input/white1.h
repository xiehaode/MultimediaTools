#ifndef WHITE1_H
#define WHITE1_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class white1;
}

class white1 : public QWidget
{
    Q_OBJECT

public:
    explicit white1(QWidget *parent = nullptr);
    ~white1();

    param getParams() const;

private:
    Ui::white1 *ui;
};

#endif // WHITE1_H
