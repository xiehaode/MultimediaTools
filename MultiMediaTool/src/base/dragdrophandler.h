#ifndef DRAGDROPHANDLER_H
#define DRAGDROPHANDLER_H

#include <QObject>
#include <QWidget>
#include <QList>
#include <QString>
#include <functional>

class DragDropHandler : public QObject
{
    Q_OBJECT

public:
    explicit DragDropHandler(QObject *parent = nullptr);

    // 绑定到目标Widget
    void attach(QWidget *widget);

    // 设置文件拖拽回调
    using FileDropCallback = std::function<void(const QList<QString>&)>;
    void setOnFileDropped(FileDropCallback callback);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QWidget *m_targetWidget = nullptr;
    FileDropCallback m_callback;
};

#endif // DRAGDROPHANDLER_H
/* 组合的使用
// 初始化拖拽处理器（组合方式）
    m_dragHandler = new DragDropHandler(this);
    m_dragHandler->attach(this);  // 绑定到当前窗口

    // 设置文件拖拽回调
    m_dragHandler->setOnFileDropped([this](const QList<QString> &filePaths) {
        this->handleDroppedFiles(filePaths);
    });

*/


/*    继承的实现
DragDropBase::DragDropBase(QWidget *parent)
    : QMainWindow(parent)
{
    this->setAcceptDrops(true);  // 启用拖拽
}

DragDropBase::~DragDropBase()
{
}

void DragDropBase::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void DragDropBase::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QString> filePaths;
        QList<QUrl> urls = mimeData->urls();

        for (const QUrl &url : urls)
        {
            QString path = url.toLocalFile();
            if (!path.isEmpty())
            {
                filePaths.append(path);
            }
        }

        // 调用子类实现的处理方法
        handleFileDrop(filePaths);
    }
}
*/
