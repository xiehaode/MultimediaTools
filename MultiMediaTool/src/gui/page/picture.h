#ifndef PICTURE_H
#define PICTURE_H

#include <QWidget>

namespace Ui {
class picture;
}

class picture : public QWidget
{
    Q_OBJECT

public:
    explicit picture(QWidget *parent = nullptr);
    ~picture();

private:
    Ui::picture *ui;
};

#endif // PICTURE_H
