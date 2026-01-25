#ifndef VIDEOPAGE_H
#define VIDEOPAGE_H

#include <QWidget>

namespace Ui {
class videoPage;
}

class videoPage : public QWidget
{
    Q_OBJECT

public:
    explicit videoPage(QWidget *parent = nullptr);
    ~videoPage();

private:
    Ui::videoPage *ui;
};

#endif // VIDEOPAGE_H
