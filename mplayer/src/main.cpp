#include <QApplication>
#include "mplayer.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mpalyer mp;
    mp.show();

    return app.exec();
}
