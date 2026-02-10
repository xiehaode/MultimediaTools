# 🚨 最小化崩溃修复方案

## 🔍 **问题确认**
- **0xC0000005 内存访问冲突**：异步线程访问已释放的 FFmpeg 资源
- **QtConcurrent 线程竞争**：多个异步任务共享同一个 AVProcessor 实例
- **野指针访问**：访问 `0x0000000000000010` (空指针偏移16字节，即 ctx->streams)

## 🔧 **最小化修复 - 无需复杂线程安全机制**

### 核心思路：为每个异步任务创建独立实例，避免共享资源

### 📋 修复步骤

#### 1. **修复 effact.cpp 中的异步调用**

```cpp
void effact::on_pushButton_clicked()
{
    if (m_importPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "未选择输入文件");
        return;
    }

    QString srcFormat = ui->comboBoxSrc->currentText().toLower();
    QString dstFormat = ui->comboBoxDst->currentText().toLower();
    
    QString outPath = QFileDialog::getSaveFileName(this, "选择输出路径", 
                                                  QDir::currentPath(), 
                                                  QString("Video Files (*.%1)").arg(dstFormat));
    
    if (outPath.isEmpty()) return;

    ui->pushButton->setEnabled(false);
    
    std::string input = QDir::toNativeSeparators(m_importPath).toUtf8().constData();
    std::string output = QDir::toNativeSeparators(outPath).toUtf8().constData();

    // 🔥 关键修复：创建独立的处理器副本
    void* processor_copy = AVProcessor_Create();
    if (!processor_copy) {
        QMessageBox::critical(this, "错误", "无法创建处理器副本");
        ui->pushButton->setEnabled(true);
        return;
    }

    QFutureWatcher<int>* watcher = new QFutureWatcher<int>(this);
    
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher, outPath, srcFormat, dstFormat, processor_copy]() {
        int result = watcher->result();
        
        // 🔥 确保异步任务完成后销毁副本
        AVProcessor_Destroy(processor_copy);
        
        if (result == 0) {
            QMessageBox::information(this, "成功", QString("文件转换成功！\n输出文件: %1").arg(outPath));
        } else {
            QString errorMsg = QString("转换失败，错误码: %1").arg(result);
            QMessageBox::critical(this, "转换失败", errorMsg);
        }

        ui->pushButton->setEnabled(true);
        watcher->deleteLater();
    });

    // 🔥 使用独立副本进行异步操作
    QFuture<int> future = QtConcurrent::run([this, processor_copy, input, output, dstFormat]() -> int {
        try {
            if (dstFormat == "gif") {
                AVConfig config;
                config.width = 480;
                config.frame_rate = 10;
                return AVProcessor_Mp4ToGif(processor_copy, input.c_str(), output.c_str(), &config);
            } else {
                // 转封装 - 使用副本处理器
                return AVProcessor_Remux(processor_copy, input.c_str(), output.c_str());
            }
        } catch (...) {
            qDebug() << "转换过程中发生异常";
            return -999;
        }
    });
    
    watcher->setFuture(future);
}
```

#### 2. **可选：简化 AVProcessor_Remux 错误处理**

如果不想修改 AVProcessor 内部，可以在 C API 层面添加简单的错误处理：

```cpp
extern "C" FORMATCHANGE_API int AVProcessor_Remux(void* processor, const char* input_path, const char* output_path)
{
    // 1. 空指针校验
    if (!processor || !input_path || !output_path) {
        return -1;
    }

    try {
        // 2. 调用成员函数
        AVProcessor* proc = static_cast<AVProcessor*>(processor);
        bool ret = proc->remux(std::string(input_path), std::string(output_path));
        return ret ? 0 : -1;
    } catch (...) {
        // 3. 异常捕获
        return -999;
    }
}
```

## 🎯 **为什么这个方案有效**

### ✅ **完全避免线程竞争**
- 每个异步任务使用独立的 AVProcessor 实例
- 无共享资源，无竞争条件

### ✅ **根除野指针访问**
- 异步任务拥有自己的 FFmpeg 上下文
- 主线程销毁 m_processor 不影响异步任务

### ✅ **资源生命周期清晰**
- 副本在任务开始时创建
- 副本在任务结束时销毁
- 无内存泄漏

## 🚀 **预期效果**

1. **立即消除 0xC0000005 崩溃**
2. **支持并发转封装操作**
3. **最小化代码变更**
4. **降低引入新bug的风险**

## 🧪 **测试方法**

1. **重现崩溃**：快速多次点击转换按钮
2. **并发测试**：同时启动多个转换任务
3. **资源监控**：检查内存使用是否正常

## 📝 **注意事项**

1. 这个方案不修改 AVProcessor 的内部实现，风险最低
2. 每个异步任务创建新实例会有轻微性能开销，但换来稳定性
3. 如果需要支持任务取消，需要额外实现

这个最小化方案应该能解决你遇到的 QtConcurrent + FFmpeg 崩溃问题！