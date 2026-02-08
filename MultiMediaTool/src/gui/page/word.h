#ifndef WORD_H
#define WORD_H

#include "src/base/pagebase.h"
#include <QWidget>
#include <QString>
#include <QMap>

#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class word;
}

class word : public QWidget
{
    Q_OBJECT
    enum type{
        pdf_type,
        word_type,
        html_type,
        ppt_type,
        image_type,
        csv_type,
        excel_type
    };
public:
    explicit word(QWidget *parent = nullptr);
    ~word();

private slots:
    void on_btnImport_clicked();
    void on_pushButton_clicked();

    void on_comboBoxSrc_currentIndexChanged(int index);

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessReadyRead();

private:
    void initContral();
    QString typeToString(type t);
    type stringToType(QString s);
    
    Ui::word *ui;
    QMap<type, QList<type>> supportMap;
    QProcess *m_process;
    QByteArray m_outputBuffer;
    QByteArray m_errorBuffer;
};


#endif // WORD_H
