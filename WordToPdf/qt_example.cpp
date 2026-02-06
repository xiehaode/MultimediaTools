/*
 * Qt使用示例 - 通用文件转换器
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QListWidget>

#ifdef _WIN32
#include <windows.h>
#endif

// 声明C函数
extern "C" {
    int convert_file(const char* input_path, const char* output_path);
    int check_file_support(const char* file_path);
    const char* get_supported_formats();
    const char* get_file_type(const char* file_path);
    char* get_last_error();
    void clear_error();
}

class ConverterWindow : public QMainWindow
{
    Q_OBJECT

public:
    ConverterWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        loadSupportedFormats();
    }

private slots:
    void selectInputFile()
    {
        QString fileName = QFileDialog::getOpenFileName(
            this, "选择输入文件", "", "所有文件 (*.*)");
        
        if (!fileName.isEmpty()) {
            m_inputFileEdit->setText(fileName);
            updateFileInfo();
            
            // 自动设置输出文件名
            QString outputName = fileName;
            QString outputExt = getOutputExtension();
            int lastDot = outputName.lastIndexOf('.');
            if (lastDot != -1) {
                outputName = outputName.left(lastDot) + outputExt;
            }
            m_outputFileEdit->setText(outputName);
        }
    }
    
    void selectOutputFile()
    {
        QString fileName = QFileDialog::getSaveFileName(
            this, "选择输出文件", m_outputFileEdit->text(), 
            QString("输出文件 (*%1)").arg(getOutputExtension()));
        
        if (!fileName.isEmpty()) {
            m_outputFileEdit->setText(fileName);
        }
    }
    
    void convertFile()
    {
        QString inputPath = m_inputFileEdit->text();
        QString outputPath = m_outputFileEdit->text();
        
        if (inputPath.isEmpty() || outputPath.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择输入和输出文件");
            return;
        }
        
        m_convertButton->setEnabled(false);
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, 0); // 不确定进度
        
        clear_error();
        
        QByteArray inputBytes = inputPath.toUtf8();
        QByteArray outputBytes = outputPath.toUtf8();
        
        int result = convert_file(inputBytes.constData(), outputBytes.constData());
        
        m_progressBar->setVisible(false);
        m_convertButton->setEnabled(true);
        
        if (result == 1) {
            QMessageBox::information(this, "成功", "文件转换成功！");
            m_logTextEdit->append(QString("✓ %1 -> %2").arg(inputPath, outputPath));
        } else {
            QString error = QString::fromUtf8(get_last_error());
            QMessageBox::critical(this, "错误", QString("文件转换失败: %1").arg(error));
            m_logTextEdit->append(QString("✗ %1 -> %2: %3").arg(inputPath, outputPath, error));
        }
    }
    
    void onOutputFormatChanged()
    {
        // 更新输出文件扩展名
        QString outputPath = m_outputFileEdit->text();
        if (!outputPath.isEmpty()) {
            QString outputExt = getOutputExtension();
            int lastDot = outputPath.lastIndexOf('.');
            if (lastDot != -1) {
                outputPath = outputPath.left(lastDot) + outputExt;
                m_outputFileEdit->setText(outputPath);
            }
        }
        
        updateConversionInfo();
    }
    
    void updateFileInfo()
    {
        QString inputPath = m_inputFileEdit->text();
        if (!inputPath.isEmpty()) {
            clear_error();
            
            QByteArray pathBytes = inputPath.toUtf8();
            int supported = check_file_support(pathBytes.constData());
            
            if (supported == 1) {
                const char* type = get_file_type(pathBytes.constData());
                QString fileType = QString::fromUtf8(type);
                
                m_fileInfoLabel->setText(QString("文件类型: %1 ✓").arg(fileType));
                m_fileInfoLabel->setStyleSheet("color: green;");
            } else {
                QString error = QString::fromUtf8(get_last_error());
                m_fileInfoLabel->setText(QString("不支持: %1").arg(error));
                m_fileInfoLabel->setStyleSheet("color: red;");
            }
            
            updateConversionInfo();
        }
    }
    
    void updateConversionInfo()
    {
        // 这里可以显示从当前文件类型到目标类型的转换信息
        // 实际实现中，可以从get_supported_formats()获取详细信息
    }

private:
    void setupUI()
    {
        QWidget *centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        // 文件选择组
        QGroupBox *fileGroup = new QGroupBox("文件选择");
        QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
        
        // 输入文件
        QHBoxLayout *inputLayout = new QHBoxLayout;
        m_inputFileEdit = new QLineEdit;
        QPushButton *inputButton = new QPushButton("浏览...");
        inputLayout->addWidget(new QLabel("输入文件:"));
        inputLayout->addWidget(m_inputFileEdit);
        inputLayout->addWidget(inputButton);
        
        // 输出格式选择
        QHBoxLayout *formatLayout = new QHBoxLayout;
        m_formatCombo = new QComboBox;
        m_formatCombo->addItem("PDF", "pdf");
        m_formatCombo->addItem("Word", "word");
        m_formatCombo->addItem("HTML", "html");
        m_formatCombo->addItem("图片", "image");
        m_formatCombo->addItem("CSV", "csv");
        formatLayout->addWidget(new QLabel("输出格式:"));
        formatLayout->addWidget(m_formatCombo);
        
        // 输出文件
        QHBoxLayout *outputLayout = new QHBoxLayout;
        m_outputFileEdit = new QLineEdit;
        QPushButton *outputButton = new QPushButton("浏览...");
        outputLayout->addWidget(new QLabel("输出文件:"));
        outputLayout->addWidget(m_outputFileEdit);
        outputLayout->addWidget(outputButton);
        
        fileLayout->addLayout(inputLayout);
        fileLayout->addLayout(formatLayout);
        fileLayout->addLayout(outputLayout);
        
        // 信息显示组
        QGroupBox *infoGroup = new QGroupBox("转换信息");
        QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
        m_fileInfoLabel = new QLabel("请选择输入文件");
        m_conversionInfoLabel = new QLabel("请选择输出格式");
        infoLayout->addWidget(m_fileInfoLabel);
        infoLayout->addWidget(m_conversionInfoLabel);
        
        // 操作按钮
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        m_convertButton = new QPushButton("开始转换");
        buttonLayout->addStretch();
        buttonLayout->addWidget(m_convertButton);
        
        // 进度条
        m_progressBar = new QProgressBar;
        m_progressBar->setVisible(false);
        
        // 日志显示
        QGroupBox *logGroup = new QGroupBox("操作日志");
        QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
        m_logTextEdit = new QTextEdit;
        m_logTextEdit->setMaximumHeight(150);
        m_logTextEdit->setReadOnly(true);
        logLayout->addWidget(m_logTextEdit);
        
        // 添加到主布局
        mainLayout->addWidget(fileGroup);
        mainLayout->addWidget(infoGroup);
        mainLayout->addWidget(m_progressBar);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(logGroup);
        
        // 连接信号
        connect(inputButton, &QPushButton::clicked, this, &ConverterWindow::selectInputFile);
        connect(outputButton, &QPushButton::clicked, this, &ConverterWindow::selectOutputFile);
        connect(m_convertButton, &QPushButton::clicked, this, &ConverterWindow::convertFile);
        connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ConverterWindow::onOutputFormatChanged);
        connect(m_inputFileEdit, &QLineEdit::textChanged, this, &ConverterWindow::updateFileInfo);
        
        // 设置窗口属性
        setWindowTitle("通用文件转换器");
        setMinimumSize(600, 400);
    }
    
    void loadSupportedFormats()
    {
        clear_error();
        const char* formats = get_supported_formats();
        QString formatsStr = QString::fromUtf8(formats);
        m_logTextEdit->append("支持的转换格式: " + formatsStr);
    }
    
    QString getOutputExtension()
    {
        QString format = m_formatCombo->currentData().toString();
        if (format == "pdf") return ".pdf";
        if (format == "word") return ".docx";
        if (format == "html") return ".html";
        if (format == "image") return ".png";
        if (format == "csv") return ".csv";
        return ".pdf";
    }

private:
    QLineEdit *m_inputFileEdit;
    QLineEdit *m_outputFileEdit;
    QComboBox *m_formatCombo;
    QPushButton *m_convertButton;
    QProgressBar *m_progressBar;
    QLabel *m_fileInfoLabel;
    QLabel *m_conversionInfoLabel;
    QTextEdit *m_logTextEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ConverterWindow window;
    window.show();
    
    return app.exec();
}

#include "qt_example.moc"