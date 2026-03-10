#ifndef REVERSE_H
#define REVERSE_H

#include <QWidget>

namespace Ui {
class reverse;
}

class reverse : public QWidget
{
    Q_OBJECT

public:
    explicit reverse(QWidget *parent = nullptr);
    ~reverse();

private:
    Ui::reverse *ui;
};

#endif // REVERSE_H
