#ifndef CONACAT_H
#define CONACAT_H

#include <QWidget>

namespace Ui {
class conacat;
}

class conacat : public QWidget
{
    Q_OBJECT

public:
    explicit conacat(QWidget *parent = nullptr);
    ~conacat();

private:
    Ui::conacat *ui;
};

#endif // CONACAT_H
