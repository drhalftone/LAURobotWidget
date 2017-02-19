#include "lautcpserialportwidget.h"
#include "laurobotwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LAUTCPSerialPortServer s;

    LAUZeroConfClientDialog b(QString("_lautcpserialportserver._tcp"));
    return b.exec();

    LAURobotWidget w;
    w.show();
    //return w.exec();

    return a.exec();
}
