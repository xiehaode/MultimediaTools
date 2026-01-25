#include "gui/application.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    MainWindow w;
    // 与 basewindow 内部设置保持一致，避免覆盖掉最小化相关 flag
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    w.resize(1000,800);
    w.show();


    return a.exec();
}
