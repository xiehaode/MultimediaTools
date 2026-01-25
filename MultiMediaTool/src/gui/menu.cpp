#include "menu.h"

Menu::Menu(QWidget *parent) :
    QWidget(parent)
{
    this->setMinimumWidth(200);
    this->setMaximumWidth(300);
    this->setAttribute(Qt::WA_StyledBackground, true);  // 启用样式表对窗口背景的控制

    newPage1BtnVideo = new QPushButton(this);
    newPage2BtnWord = new QPushButton(this);
    newPage3BtnVideoEffact = new QPushButton(this);
    newPage4BtnPicture = new QPushButton(this);

    // 设置样式表，包括背景颜色
//    QString styleSheet = "background-color: #A0A083;"
//                         "border-radius: 7px;";
//    this->setStyleSheet(styleSheet);

    init();



    // 连接信号与槽，当按钮选中时调用自定义的槽函数
    //connect(&buttonGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &Menu::onButtonClicked);
    connect(newPage1BtnVideo, &QPushButton::clicked, this, [this]() {
        for(QAbstractButton *b : buttonGroup.buttons()) {
            if (b != newPage1BtnVideo) {
                b->setStyleSheet("background-color: transparent;"
                                    "opacity: 0;"
                                    "border-radius: 7px;");
            }
        }

        // 更改选中按钮的样式
        if (newPage1BtnVideo) {
            newPage1BtnVideo->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);"
                                  "border-radius: 7px;");
        }
    });

    connect(newPage2BtnWord, &QPushButton::clicked, this, [this]() {
        for(QAbstractButton *b : buttonGroup.buttons()) {
            if (b != newPage2BtnWord) {
                b->setStyleSheet("background-color: transparent;"
                                    "opacity: 0;"
                                    "border-radius: 7px;");
            }
        }

        // 更改选中按钮的样式
        if (newPage2BtnWord) {
            newPage2BtnWord->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);"
                                  "border-radius: 7px;");
        }
    });

    connect(newPage3BtnVideoEffact, &QPushButton::clicked, this, [this]() {
        for(QAbstractButton *b : buttonGroup.buttons()) {
            if (b != newPage3BtnVideoEffact) {
                b->setStyleSheet("background-color: transparent;"
                                    "opacity: 0;"
                                    "border-radius: 7px;");
            }
        }

        // 更改选中按钮的样式
        if (newPage3BtnVideoEffact) {
            newPage3BtnVideoEffact->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);"
                                  "border-radius: 7px;");
        }
    });

    connect(newPage4BtnPicture, &QPushButton::clicked, this, [this]() {
        for(QAbstractButton *b : buttonGroup.buttons()) {
            if (b != newPage4BtnPicture) {
                b->setStyleSheet("background-color: transparent;"
                                    "opacity: 0;"
                                    "border-radius: 7px;");
            }
        }

        // 更改选中按钮的样式
        if (newPage4BtnPicture) {
            newPage4BtnPicture->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);"
                                  "border-radius: 7px;");
        }
    });
}

void Menu::init()
{
    auto *layout = new QVBoxLayout;



    auto *titleLayout = new QHBoxLayout;


    layout->addLayout(titleLayout);

    // 创建菜单栏按钮控件
    QString video = GBK2QString("视频");
    setPushButton(newPage1BtnVideo, QString::fromUtf8(":/rc/video.svg"), video);
    QString word = GBK2QString("文档");
    setPushButton(newPage2BtnWord, QString::fromUtf8(":/rc/word.svg"), word);
    QString effact = GBK2QString("视频特效");
    setPushButton(newPage3BtnVideoEffact, QString::fromUtf8(":/rc/effact.svg"), effact);
    QString picture = GBK2QString("图片处理");
    setPushButton(newPage4BtnPicture, QString::fromUtf8(":/rc/picture.svg"), picture);

    // 创建按钮组
    buttonGroup.addButton(newPage1BtnVideo, 1);
    buttonGroup.addButton(newPage2BtnWord, 2);
    buttonGroup.addButton(newPage3BtnVideoEffact, 3);
    buttonGroup.addButton(newPage4BtnPicture,4);

    // 使用样式表设置控件的背景透明和高度增加 字体设置
    QFont font("Microsoft YaHei", 10);              // 创建字体，指定字体名称和字体大小
    font.setPointSizeF(font.pointSizeF() * 1.25);                    // 将字体大小放大1.25倍

    foreach(QAbstractButton *b, buttonGroup.buttons()) {
        b->setStyleSheet("background-color: transparent;"
                             "opacity: 0;"
                             "border-radius: 7px;");
        // 设置按钮的高度和字体
        b->setFixedHeight(40);
        b->setFont(font);
    }

    // 设置第一个按钮为默认选中状态
    if (QAbstractButton *firstButton = buttonGroup.button(1)) {
        firstButton->setChecked(true);
        onButtonClicked(1);
    }

    // 创建布局
    layout->addWidget(newPage1BtnVideo);
    layout->addWidget(newPage2BtnWord);
    layout->addWidget(newPage3BtnVideoEffact);
    layout->addWidget(newPage4BtnPicture);


    // 添加一个竖直方向的额弹簧
    auto *spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(spacer);
    // 设置布局
    this->setLayout(layout);
}

void Menu::onButtonClicked(int id)
{
    // 获取选中的按钮
    auto *button = qobject_cast<QPushButton*>(buttonGroup.button(id));

    // 将所有按钮的样式表恢复到初始状态
    for(QAbstractButton *b : buttonGroup.buttons()) {
        if (b != button) {
            b->setStyleSheet("background-color: transparent;"
                                "opacity: 0;"
                                "border-radius: 7px;");
        }
    }

    // 更改选中按钮的样式
    if (button) {
        button->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);"
                              "border-radius: 7px;");

        emit buttonClicked(id);
    }
}

void Menu::setPushButton(QPushButton *button, const QString& url, const QString& text)
{
    // 设置按钮的文字和图标
    button->setText(text);
    QIcon icon;
    // 图标大小设置为30*30
    icon.addFile(url, QSize());
    button->setIcon(icon);
    button->setIconSize(QSize(30, 30));

    // 设置按钮光标样式
    button->setCursor(QCursor(Qt::PointingHandCursor));
}

void Menu::setPushButton(QPushButton *button, const QString& text)
{
    button->setText(text);
}

void Menu::onMinimizeButtonClicked()
{
    isMinimized = !isMinimized;
    setMenu(isMinimized);
}

void Menu::setMenu(bool isMini)
{
    // 最小化
    if (isMini)
    {
        // 设置最小宽度和最大宽度
        this->setMinimumWidth(50);
        this->setMaximumWidth(50);

        newPage1BtnVideo->setText("视频");
        newPage2BtnWord->setText("文档");
        newPage3BtnVideoEffact->setText("视频效果");
        newPage4BtnPicture->setText("图片处理");

        // 设置按钮的图标
    }
    // 正常
    else {
        // 设置最小宽度和最大宽度
        this->setMinimumWidth(200);
        this->setMaximumWidth(300);

        newPage1BtnVideo->setText(tr("First page"));
        newPage2BtnWord->setText(tr("Second page"));
        newPage3BtnVideoEffact->setText(tr("Third page"));
        newPage4BtnPicture->setText("图片处理");

    }
}
