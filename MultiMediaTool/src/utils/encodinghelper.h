// encodinghelper.h
#ifndef ENCODINGHELPER_H
#define ENCODINGHELPER_H

#include <QString>
#include <QTextCodec>

#ifdef Q_OS_WIN
// GBK字节转QString，windows下可用
inline QString GBK2QString(const QByteArray &gbkData)
{
    static QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
    return gbkCodec ? gbkCodec->toUnicode(gbkData) : QString::fromLocal8Bit(gbkData);
}

// QString转GBK字节
inline QByteArray QString2GBK(const QString &text)
{
    static QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
    return gbkCodec ? gbkCodec->fromUnicode(text) : text.toLocal8Bit();
}
#else
// 非Windows用UTF-8
inline QString GBK2QString(const QByteArray &gbkData) { return QString::fromUtf8(gbkData); }
inline QByteArray QString2GBK(const QString &text) { return text.toUtf8(); }
#endif

#endif // ENCODINGHELPER_H
