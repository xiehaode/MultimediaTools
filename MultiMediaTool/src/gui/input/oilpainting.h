#ifndef OILPAINTING_H
#define OILPAINTING_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class oilPainting;
}

class oilPainting : public QWidget
{
    Q_OBJECT

public:
    explicit oilPainting(QWidget *parent = nullptr);
    ~oilPainting();

    param getParams() const;

private:
    Ui::oilPainting *ui;
};

#endif // OILPAINTING_H
