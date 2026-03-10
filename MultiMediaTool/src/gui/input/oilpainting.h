#ifndef OILPAINTING_H
#define OILPAINTING_H

#include <QWidget>

namespace Ui {
class oilPainting;
}

class oilPainting : public QWidget
{
    Q_OBJECT

public:
    explicit oilPainting(QWidget *parent = nullptr);
    ~oilPainting();

private:
    Ui::oilPainting *ui;
};

#endif // OILPAINTING_H
