#ifndef CONCAT_H
#define CONCAT_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QTableWidgetItem>
#include <QDebug>

namespace Ui {
class concat;
}

class concat : public QWidget
{
    Q_OBJECT

public:
    explicit concat(QWidget *parent = nullptr);
    ~concat();

private slots:
    void on_btn_Refresh_clicked();
    void on_btn_Merge_clicked();
    void on_btn_Split_clicked();
    void on_btn_Resize_clicked();
    void on_tableWidget_itemChanged(QTableWidgetItem *item);

private:
    void loadVideoFiles();
    void updateButtonStates();
    QString getVideoDir();
    QStringList getSelectedVideos();

    Ui::concat *ui;
    void* m_worker;
    QStringList m_videoFiles;
};

#endif // CONCAT_H
