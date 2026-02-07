#include "word.h"
#include "ui_word.h"

#include <QTextCodec>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

word::word(QWidget *parent) :

    ui(new Ui::word),
    m_process(new QProcess(this))
{
    ui->setupUi(this);
    initContral();
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &word::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &word::onProcessReadyRead);
    connect(m_process, &QProcess::readyReadStandardError, this, &word::onProcessReadyRead);
}

word::~word()
{
    if(m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
    delete ui;
}

void word::initContral()
{
    // 初始化映射关系
    supportMap[pdf_type] << word_type << html_type << image_type << csv_type;
    supportMap[word_type] << pdf_type << html_type << csv_type;
    supportMap[html_type] << word_type << pdf_type;
    supportMap[ppt_type] << pdf_type;
    supportMap[image_type] << pdf_type << html_type;
    supportMap[csv_type] << pdf_type << html_type << excel_type;

    // 初始化下拉框
    ui->comboBoxSrc->clear();
    ui->comboBoxSrc->addItem("PDF", pdf_type);
    ui->comboBoxSrc->addItem("Word", word_type);
    ui->comboBoxSrc->addItem("HTML", html_type);
    ui->comboBoxSrc->addItem("PPT", ppt_type);
    ui->comboBoxSrc->addItem("Image", image_type);
    ui->comboBoxSrc->addItem("CSV/Excel", csv_type);

    // 默认选择第一个并更新目标下拉框
    on_comboBox_currentIndexChanged(0);
}

void word::on_comboBox_currentIndexChanged(int index)
{
    ui->comboBoxDst->clear();
    type selectedType = (type)ui->comboBoxSrc->currentData().toInt();
    
    QList<type> targets = supportMap.value(selectedType);
    for(type t : targets) {
        ui->comboBoxDst->addItem(typeToString(t), t);
    }
}

QString word::typeToString(type t)
{
    switch(t) {
        case pdf_type: return "pdf";
        case word_type: return "word";
        case html_type: return "html";
        case ppt_type: return "pdf"; // PPT 备用转 PDF
        case image_type: return "image";
        case csv_type: return "csv";
        case excel_type: return "xlsx";
        default: return "pdf";
    }
}

void word::on_btnImport_clicked()
{
    QString inputPath = QFileDialog::getOpenFileName(this, QStringLiteral("选择源文件"), "", "All Files (*.*)");
    if(inputPath.isEmpty()) return;

    ui->lineEdit_Path->setText(inputPath);
    
    // 自动识别后缀并设置源类型
    QString ext = QFileInfo(inputPath).suffix().toLower();
    int foundIndex = -1;
    
    if(ext == "pdf") foundIndex = ui->comboBoxSrc->findData(pdf_type);
    else if(ext == "docx" || ext == "doc") foundIndex = ui->comboBoxSrc->findData(word_type);
    else if(ext == "html" || ext == "htm") foundIndex = ui->comboBoxSrc->findData(html_type);
    else if(ext == "pptx" || ext == "ppt") foundIndex = ui->comboBoxSrc->findData(ppt_type);
    else if(ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp") foundIndex = ui->comboBoxSrc->findData(image_type);
    else if(ext == "csv" || ext == "xlsx" || ext == "xls") foundIndex = ui->comboBoxSrc->findData(csv_type);

    if(foundIndex != -1) {
        ui->comboBoxSrc->setCurrentIndex(foundIndex);
        ui->listWidget->addItem(QStringLiteral("已导入: ") + QFileInfo(inputPath).fileName());
    } else {
        ui->listWidget->addItem(QStringLiteral("警告: 无法识别的文件格式，请手动选择源类型"));
    }
}

void word::on_pushButton_clicked()
{
    QString inputPath = ui->lineEdit_Path->text();
    if(inputPath.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先导入需要转换的文件！"));
        return;
    }

    // 默认输出目录设置为 exe 所在目录下的 document 文件夹
    QString defaultOutDir = QCoreApplication::applicationDirPath() + "/document";
    QDir dir(defaultOutDir);
    if (!dir.exists()) {
        dir.mkpath("."); // 如果不存在则自动创建
    }

    QString outputDir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择保存目录"), defaultOutDir);
    if(outputDir.isEmpty()) return;



    QString fileName = QFileInfo(inputPath).baseName();
    type targetType = (type)ui->comboBoxDst->currentData().toInt();
    QString ext = typeToString(targetType);
    if(targetType == word_type) ext = "docx"; // 纠正扩展名
    
    QString outputPath = outputDir + "/" + fileName + "." + ext;

    // 构造命令行参数
    QString program = QCoreApplication::applicationDirPath() + "/converter_cli.exe";
    QStringList arguments;
    arguments << "--input" << inputPath << "--output" << outputPath;

    // 解决乱码：强制 Python 使用 UTF-8 编码输出
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "GBK");
    m_process->setProcessEnvironment(env);

    ui->listWidget->addItem(QStringLiteral("开始转换: ") + QFileInfo(inputPath).fileName() + " -> " + ext);

    ui->pushButton->setEnabled(false);
    
    m_process->start(program, arguments);
}

void word::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->pushButton->setEnabled(true);
    if(exitStatus == QProcess::NormalExit && exitCode == 0) {
        ui->listWidget->addItem(" 转换成功");
        QMessageBox::information(this, "成功", "文件转换完成！");
    } else {
        ui->listWidget->addItem(" 转换失败 (代码: " + QString::number(exitCode) + ")");
        QMessageBox::critical(this, "错误", "文件转换失败，请检查控制台输出或日志。");
    }
}

void word::onProcessReadyRead()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    
    QByteArray outData = m_process->readAllStandardOutput();
    if(!outData.isEmpty()) {
        ui->listWidget->addItem("OUT: " + codec->toUnicode(outData).trimmed());
    }
    
    QByteArray errData = m_process->readAllStandardError();
    if(!errData.isEmpty()) {
        ui->listWidget->addItem("ERR: " + codec->toUnicode(errData).trimmed());
    }
}


