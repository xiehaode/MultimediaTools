#ifndef MOSAIC_H
#define MOSAIC_H

#include <QWidget>

namespace Ui {
class mosaic;
}

class mosaic : public QWidget
{
    Q_OBJECT

public:
    explicit mosaic(QWidget *parent = nullptr);
    ~mosaic();

private:
    Ui::mosaic *ui;
};

#endif // MOSAIC_H
