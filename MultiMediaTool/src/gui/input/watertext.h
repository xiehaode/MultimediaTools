#ifndef WATERTEXT_H
#define WATERTEXT_H

#include <QWidget>
#include "OpenCVFFMpegTools.h"

namespace Ui {
class waterText;
}

class waterText : public QWidget
{
    Q_OBJECT

public:
    explicit waterText(QWidget *parent = nullptr);
    ~waterText();

    param getParams() const;

private:
    Ui::waterText *ui;
};

#endif // WATERTEXT_H
