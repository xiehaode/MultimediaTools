#include "src/base/dragdrophandler.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

DragDropHandler::DragDropHandler(QObject *parent)
    : QObject(parent)
{
}

void DragDropHandler::attach(QWidget *widget)
{
    m_targetWidget = widget;
    m_targetWidget->setAcceptDrops(true);
    m_targetWidget->installEventFilter(this);  // 安装事件过滤器
}

void DragDropHandler::setOnFileDropped(FileDropCallback callback)
{
    m_callback = callback;
}

bool DragDropHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_targetWidget)
        return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::DragEnter)
    {
        QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
        if (dragEvent->mimeData()->hasUrls())
        {
            dragEvent->acceptProposedAction();
            return true;
        }
    }
    else if (event->type() == QEvent::Drop)
    {
        QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
        if (dropEvent->mimeData()->hasUrls())
        {
            QList<QString> filePaths;
            QList<QUrl> urls = dropEvent->mimeData()->urls();

            for (const QUrl &url : urls)
            {
                QString path = url.toLocalFile();
                if (!path.isEmpty())
                {
                    filePaths.append(path);
                }
            }

            if (m_callback)
            {
                m_callback(filePaths);  // 调用回调
            }

            dropEvent->acceptProposedAction();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
