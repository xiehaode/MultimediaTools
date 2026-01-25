#include "application.h"

MainWindow::MainWindow(QWidget *parent)
    : basewindow(parent)
{
    // 创建QStackedWidget控件 用于存放多个页面
        qStackedWidget = new QStackedWidget;
        qStackedWidget->setObjectName("BaseContentWidget");
//        auto *newPage1 = new NewPage1;
//        auto *newPage2 = new NewPage2;
//        auto *newPage3 = new NewPage3;

//        qStackedWidget->addWidget(newPage1);
//        qStackedWidget->addWidget(newPage2);
//        qStackedWidget->addWidget(newPage3);

        this->createleftNavigationBar();

        // 绑定侧边栏按钮和主界面的控件
        connect(sidebarWidget, SIGNAL(buttonClicked(int)), this, SLOT(onButtonClicked(int)));
}

bool MainWindow::createleftNavigationBar()
{
    // 设置窗口最小大小为1000x700
        this->setMinimumSize(QSize(1000, 700));
        // 启用样式表对窗口背景的控制
        this->setAttribute(Qt::WA_StyledBackground, true);
        // 设置窗口标题为"musicPlayer"
        this->setWindowTitle(tr("demo_menu"));

        // 设置窗口的样式表为蓝色背景
        QString styleSheet = QString("#BaseContentWidget{background-color: #2177B8}");
        this->setStyleSheet(styleSheet);

        /* 侧边栏设置 */
        // 创建左侧边栏容器
        sidebarWidget = new Menu;
        // 创建透明效果SidebarWidget
        auto *opacityEffect = new QGraphicsOpacityEffect();
        // 设置透明度为0.9（90%不透明）
        opacityEffect->setOpacity(0.9);
        // 应用透明度效果
        sidebarWidget->setGraphicsEffect(opacityEffect);

        /* 主内容容器设置 */
        // 创建内容布局
        auto *contentLayout = new QVBoxLayout;

        // 创建内容容器
        content = new QWidget;
        contentLayout->addWidget(qStackedWidget);  // 添加qStackedWidget到content
        content->setLayout(contentLayout);         // 设置content的布局为contentLayout

        // 创建页面左右分割QSplitter控件，分割左侧边栏和主内容容器
        QSplitter *splitter = createVerticalSplitter(this, sidebarWidget, content,
                                                     Qt::Horizontal, 200, 600);

        auto *layout = new QVBoxLayout;
        // 将分割窗口添加到主布局中
        layout->addWidget(splitter);

        // 设置整个窗口的布局为主布局
        //获取内容区的窗口
        QWidget* Conten =  getContentWidget();
        Conten->setLayout(layout);
        return true;
}

MainWindow::~MainWindow()
{

}


QSplitter* MainWindow::createVerticalSplitter(QWidget *parent, QWidget *content1, QWidget *content2,
                                        Qt::Orientation orientation, int size1, int size2) {
    // 创建右边页面的QSplitter控件
    auto *splitter = new QSplitter(orientation, parent);
    splitter->addWidget(content1);                          // 添加左侧边栏到分割窗口中
    splitter->addWidget(content2);                          // 添加内容容器到分割窗口中
    // 设置初始高度
    splitter->setSizes(QList<int>() << size1 << size2);
    // 获取分割窗口的句柄
    QSplitterHandle *splitterHandle1 = splitter->handle(1);  // 获取右侧部分的句柄
    // 设置句柄样式，并禁用拖动功能
    splitterHandle1->setEnabled(false);

    return splitter;
}

void MainWindow::onButtonClicked(int buttonId)
{
    int pageIndex = buttonId - 1;                           // 索引从0开始
    qStackedWidget->setCurrentIndex(pageIndex);
}
