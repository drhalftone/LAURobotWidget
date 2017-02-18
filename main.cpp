#include "lautcpserialportwidget.h"
#include "laurobotwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LAURobotWidget w;
    w.show();
    //return w.exec();

    return a.exec();
}
