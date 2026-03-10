#ifndef GREY_H
#define GREY_H

#include <QWidget>

namespace Ui {
class grey;
}

class grey : public QWidget
{
    Q_OBJECT

public:
    explicit grey(QWidget *parent = nullptr);
    ~grey();

private:
    Ui::grey *ui;
};

#endif // GREY_H
