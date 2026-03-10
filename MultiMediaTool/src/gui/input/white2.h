#ifndef WHITE2_H
#define WHITE2_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class white2;
}

class white2 : public QWidget
{
    Q_OBJECT

public:
    explicit white2(QWidget *parent = nullptr);
    ~white2();

    param getParams() const;

private:
    Ui::white2 *ui;
};

#endif // WHITE2_H
