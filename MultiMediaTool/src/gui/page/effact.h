#ifndef EFFACT_H
#define EFFACT_H
#include "src/base/pagebase.h"
#include <QWidget>

namespace Ui {
class effact;
}

class effact : public QWidget
{
    Q_OBJECT

public:
    explicit effact(QWidget *parent = nullptr);
    ~effact();

private:
    Ui::effact *ui;
};

#endif // EFFACT_H
