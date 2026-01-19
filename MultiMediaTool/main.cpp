#include "MultiMediaTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	
    QApplication a(argc, argv);
    MultiMediaTool w;
    w.show();
    return a.exec();
}
