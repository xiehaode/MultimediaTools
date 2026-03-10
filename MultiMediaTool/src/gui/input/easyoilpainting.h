#ifndef EASYOILPAINTING_H
#define EASYOILPAINTING_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class easyOilPainting;
}

class easyOilPainting : public QWidget
{
    Q_OBJECT

public:
    explicit easyOilPainting(QWidget *parent = nullptr);
    ~easyOilPainting();

    param getParams() const;

private:
    Ui::easyOilPainting *ui;
};

#endif // EASYOILPAINTING_H
