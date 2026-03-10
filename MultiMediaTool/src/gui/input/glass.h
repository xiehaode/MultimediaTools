#ifndef GLASS_H
#define GLASS_H

#include <QWidget>

namespace Ui {
class glass;
}

class glass : public QWidget
{
    Q_OBJECT

public:
    explicit glass(QWidget *parent = nullptr);
    ~glass();

private:
    Ui::glass *ui;
};

#endif // GLASS_H
