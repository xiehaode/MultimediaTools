#ifndef MVERSION_H
#define MVERSION_H

#include <QWidget>

namespace Ui {
class version;
}

class mVersion : public QWidget
{
    Q_OBJECT

public:
    explicit mVersion(QWidget *parent = nullptr);
    ~mVersion();

private:
    Ui::version *ui;
};

#endif // MVERSION_H
