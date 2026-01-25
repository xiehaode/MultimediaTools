#ifndef WORD_H
#define WORD_H

#include <QWidget>

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
