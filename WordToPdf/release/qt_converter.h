
#ifndef QT_CONVERTER_H
#define QT_CONVERTER_H

#include <QString>
#include <QStringList>
#include <QDebug>

// ÉùÃ÷Cº¯Êý
extern "C" {
    int convert_file(const char* input_path, const char* output_path);
    int convert_files_batch(const char* const* input_paths, int file_count, 
                           const char* output_dir, const char* output_format);
    int check_file_support(const char* file_path);
    const char* get_supported_formats();
    const char* get_file_type(const char* file_path);
    char* get_last_error();
    void clear_error();
}

class QtFileConverter : public QObject
{
    Q_OBJECT

public:
    explicit QtFileConverter(QObject *parent = nullptr) : QObject(parent) {}

    bool convertFile(const QString &inputPath, const QString &outputPath) {
        clear_error();
        QByteArray inputBytes = inputPath.toUtf8();
        QByteArray outputBytes = outputPath.toUtf8();
        
        int result = convert_file(inputBytes.constData(), outputBytes.constData());
        
        if (!result) {
            m_lastError = QString::fromUtf8(get_last_error());
            qWarning() << "×ª»»Ê§°Ü:" << m_lastError;
        }
        
        return result == 1;
    }

    bool isFileSupported(const QString &filePath) {
        clear_error();
        QByteArray pathBytes = filePath.toUtf8();
        
        int result = check_file_support(pathBytes.constData());
        
        if (!result) {
            m_lastError = QString::fromUtf8(get_last_error());
        }
        
        return result == 1;
    }

    QString getFileType(const QString &filePath) {
        clear_error();
        QByteArray pathBytes = filePath.toUtf8();
        
        const char* type = get_file_type(pathBytes.constData());
        return QString::fromUtf8(type);
    }

    QString getSupportedFormats() {
        clear_error();
        const char* formats = get_supported_formats();
        return QString::fromUtf8(formats);
    }

    QString getLastError() const {
        return m_lastError;
    }

private:
    QString m_lastError;
};

#endif // QT_CONVERTER_H
