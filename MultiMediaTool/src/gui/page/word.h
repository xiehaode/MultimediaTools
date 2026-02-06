#ifndef WORD_H
#define WORD_H

#include "src/base/pagebase.h"
#include <QWidget>
#include <QString>

namespace Ui {
class word;
}

class word : public QWidget
{
    Q_OBJECT

public:
    explicit word(QWidget *parent = nullptr);
    ~word();

private:
    Ui::word *ui;

};

#endif // WORD_H
