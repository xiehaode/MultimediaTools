#include "basewindow.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QFile>

basewindow::basewindow(QWidget *parent)
    : QMainWindow(parent)
{
    // FramelessWindowHint属性设置窗口去除边框;
    // WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口;
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    // 设置窗口背景透明;
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("BaseWindowCentralWidget"); // 方便样式表控制
    centralWidget->setMouseTracking(false);
    this->setCentralWidget(centralWidget);

    // 注意！！！如果是主窗口不要设置WA_DeleteOnClose属性;
    // 关闭窗口时释放资源;
    //setAttribute(Qt::WA_DeleteOnClose);
    // 初始化标题栏;
    initTitleBar();


    //设置布局，隔离标题栏和内容区 ==========
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 清除布局默认边距
    mainLayout->setSpacing(0); // 标题栏和内容区之间无间距
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize); // 布局大小固定,不能设置该属性
    // 将标题栏添加到布局顶部
    mainLayout->addWidget(m_titleBar);

    // 创建专门的内容区部件
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("BaseWindowContentWidget");
    // 设置内容区背景
    m_contentWidget->setStyleSheet("QWidget#BaseWindowContentWidget { background-color: #f5f5f5; }");

    // 将内容区添加到布局，占满剩余空间
    mainLayout->addWidget(m_contentWidget, 1); // 第二个参数1表示占满剩余空间

    //为标题栏和内容区安装事件过滤器
    m_titleBar->installEventFilter(this);
    m_contentWidget->installEventFilter(this);
    centralWidget->installEventFilter(this);
}


bool basewindow::eventFilter(QObject *watched, QEvent *event)
{
    // 拦截所有鼠标按下/移动/释放事件
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonRelease)
    {
        // 匹配标题栏、内容区、中心部件
        if (watched == m_titleBar || watched == m_contentWidget ||
            watched == this->centralWidget())
        {
            event->accept(); // 接受事件，不再向上/向下传递
            return true;     // 过滤掉该事件，不触发任何后续处理
        }
    }
    return QMainWindow::eventFilter(watched, event);
}






basewindow::~basewindow()
{

}

void basewindow::initTitleBar()
{
    m_titleBar = new myTitleBar(this);
    m_titleBar->move(0, 0);

    connect(m_titleBar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonMinClicked()));
    connect(m_titleBar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonRestoreClicked()));
    connect(m_titleBar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonMaxClicked()));
    connect(m_titleBar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));

}

void basewindow::paintEvent(QPaintEvent* event)
{
    //设置背景色;
    QPainter painter(this);
    QPainterPath pathBack;
    pathBack.setFillRule(Qt::WindingFill);
    pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(pathBack, QBrush(QColor(238, 223, 204)));

    return QMainWindow::paintEvent(event);
}

void basewindow::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/Resources/" + sheetName + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString styleSheet = this->styleSheet();
        styleSheet += QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }
}

void basewindow::onButtonMinClicked()
{
    showMinimized();
}

void basewindow::onButtonRestoreClicked()
{
    QPoint windowPos;
    QSize windowSize;
    m_titleBar->getRestoreInfo(windowPos, windowSize);
    this->setGeometry(QRect(windowPos, windowSize));
}

void basewindow::onButtonMaxClicked()
{
    m_titleBar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    setGeometry(FactRect);
}

void basewindow::onButtonCloseClicked()
{
    close();
}

QWidget* basewindow::getContentWidget()
{
    return m_contentWidget;
}
