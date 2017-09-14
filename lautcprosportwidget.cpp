#include "lautcprosportwidget.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPortServer::LAUTCPROSPortServer(int num, unsigned short identifier, QObject *parent) : QObject(parent)
{
    // DROP IN DEFAULT PORT NUMBER IF USER SUPPLIED VALUE IS NEGATIVE
    if (num < 100) {
        num = LAUTCPSERIALPORTSERVERPORTNUMER;
    }

    // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
    QList<QROSPortInfo> portList = QROSPortInfo::availablePorts();
    for (int m = 0; m < portList.count(); m++) {
        qDebug() << portList.at(m).portName();
        qDebug() << portList.at(m).productIdentifier();
        if (identifier == 0xFFFF || portList.at(m).productIdentifier() == identifier) {
            ports << new LAUTCPROSPort(portList.at(m).portName(), num + m);
        }
        for (int n = portList.count() - 1; n > m; n--) {
            if (portList.at(n).description() == portList.at(m).description() &&
                portList.at(n).manufacturer() == portList.at(m).manufacturer() &&
                portList.at(n).vendorIdentifier() == portList.at(m).vendorIdentifier() &&
                portList.at(n).productIdentifier() == portList.at(m).productIdentifier()) {
                portList.removeAt(n);
            }
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPortServer::~LAUTCPROSPortServer()
{
    while (ports.count() > 0) {
        delete ports.takeFirst();
    }
}
