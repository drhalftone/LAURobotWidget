/**************************************************************************************************
    Copyright (C) 2017 Dr. Daniel L. Lau
    This file is part of LAURobotWidget.

    LAURobotWidget is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LAURobotWidget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LAURobotWidget.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************************************/
#include "lautcpserialportwidget.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPortServer::LAUTCPSerialPortServer(int num, unsigned short identifier, QString idstring, QObject *parent) : QObject(parent)
{
    // DROP IN DEFAULT PORT NUMBER IF USER SUPPLIED VALUE IS NEGATIVE
    if (num < 100) {
        num = LAUTCPSERIALPORTSERVERPORTNUMER;
    }
    if (idstring.length() < 1)
    {
        idstring = "_lautcpserialportserver._tcp";
    }
    // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (int m = 0; m < portList.count(); m++) {
        qDebug() << portList.at(m).portName();
        qDebug() << portList.at(m).productIdentifier();
        if (identifier == 0xFFFF || portList.at(m).productIdentifier() == identifier) {
            ports << new LAUTCPSerialPort(portList.at(m).portName(), num + m);
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
LAUTCPSerialPortServer::~LAUTCPSerialPortServer()
{
    while (ports.count() > 0) {
        delete ports.takeFirst();
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPort::LAUTCPSerialPort(QString string, int prtNmbr, QString idstring, QObject *parent) : QTcpServer(parent), connected(false), portNumber(prtNmbr), portString(string), port(NULL), socket(NULL), zeroConf(NULL)
{
    // SET THE SERIAL PORT SETTINGS
    port.setPortName(portString);
    port.setBaudRate(QSerialPort::Baud38400);
    port.setDataBits(QSerialPort::Data8);
    port.setStopBits(QSerialPort::OneStop);
    port.setParity(QSerialPort::NoParity);
    port.setFlowControl(QSerialPort::NoFlowControl);

    // CONNECT THE SERIAL PORT TO THE READY READ SLOT
    connect(&port, SIGNAL(readyRead()), this, SLOT(onReadyReadSerial()));

    // CREATE A ZERO CONF INSTANCE AND ADVERTISE THE SERIAL PORT
    zeroConf = new QZeroConf();
    connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
    connect(zeroConf, SIGNAL(servicePublished()), this, SLOT(onServicePublished()));
    if (idstring.length() < 1)
    {
        idstring = "_lautcpserialportserver._tcp";
    }
    zeroConf->startServicePublish(portString.toUtf8(), idstring, "local", portNumber);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPort::~LAUTCPSerialPort()
{
    if (port.isOpen()) {
        port.close();
    }
    if (socket) {
        delete socket;
    }
    if (zeroConf) {
        delete zeroConf;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::incomingConnection(qintptr handle)
{
    // OPEN THE SERIAL PORT FOR COMMUNICATION
    if (!port.open(QIODevice::ReadWrite)) {
        emit emitError(QString("Cannot connect to serial port.\n") + port.errorString());
    } else if (!port.isReadable()) {
        emit emitError(QString("Port is not readable!\n") + port.errorString());
    } else {
        // SET THE CONNECTED FLAG HIGH
        connected = true;

        // CALL THE BASE CLASS TO USE ITS DEFAULT METHOD
        QTcpServer::incomingConnection(handle);

        // NOW LET'S CREATE THE SOCKET FOR MESSAGE HANDLING TO THE TRACKER
        socket = this->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyReadTCP()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

        QHostAddress hostAddress = socket->peerAddress();
        clientIPAddress = hostAddress.toString();
        qDebug() << "LAU3DVideoTCPServer :: Accepting incoming connection from " << clientIPAddress;

        // STOP LISTENING FOR NEW CONNECTIONS
        this->close();
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onReadyReadTCP()
{
    // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
    QByteArray byteArray = socket->readAll();
    if (byteArray.isEmpty() == false) {
        qDebug() << "LAUTCPSerialPort" << byteArray;

        // SEND THE BYTES TO THE SERIAL PORT
        port.write(byteArray);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onReadyReadSerial()
{
    // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
    QByteArray byteArray = port.readAll();
    if (byteArray.isEmpty() == false) {
        // SEND THE BYTES TO THE TCP CLIENT
        socket->write(byteArray);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onDisconnected()
{
    // CLOSE THE SERIAL PORT CONNECTION
    port.close();

    // DELETE SOCKET TO CLOSE CONNECTION
    if (socket) {
        socket->deleteLater();
        socket = NULL;
    }
    connected = false;

    // START LISTENING FOR A NEW CONNECTION
    if (!this->isListening()) {
        if (!this->listen(QHostAddress::Any, portNumber)) {
            qDebug() << "LAUTCPSerialPort :: Error trying to listen for incoming connections!";
        }
    }
    qDebug() << "LAUTCPSerialPort :: Closing connection from " << clientIPAddress;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onServicePublished()
{
    if (!this->listen(QHostAddress::Any, portNumber)) {
        qDebug() << "LAUTCPSerialPort ::" << portString << ":: Error trying to listen for incoming connections!";
    } else {
        qDebug() << "LAUTCPSerialPort ::" << portString << ":: Listening for new connections!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onServiceError(QZeroConf::error_t error)
{
#ifdef LAU_CLIENT
    switch (error) {
        case QZeroConf::noError:
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "LAU3DVideoTCPServer :: Remote host closed error!";
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "LAU3DVideoTCPServer :: Host not found error!";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "LAU3DVideoTCPServer :: Connection refused error!";
            break;
        default:
            qDebug() << "LAU3DVideoTCPClient :: Default error!";
    }
}

#ifdef LAU_CLIENT
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUTCPSerialPortClient::LAUTCPSerialPortClient(QString portString, QObject *parent) : QObject(parent), ipAddress(portString), portNumber(-1), port(NULL)
{
    if (portString.isEmpty()) {
        // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
        QStringList ports;
        QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
        for (int m = 0; m < portList.count(); m++) {
            ports << portList.at(m).portName();
            for (int n = portList.count() - 1; n > m; n--) {
                if (portList.at(n).description() == portList.at(m).description() &&
                    portList.at(n).manufacturer() == portList.at(m).manufacturer() &&
                    portList.at(n).vendorIdentifier() == portList.at(m).vendorIdentifier() &&
                    portList.at(n).productIdentifier() == portList.at(m).productIdentifier()) {
                    portList.removeAt(n);
                }
            }
        }

        // ASK THE USER WHICH PORT SHOULD WE USE AND THEN TRY TO CONNECT
        bool okay = false;
        portString = QInputDialog::getItem(NULL, QString("LAUTCPSerialPortClient"), QString("Select Serial Port"), ports, 0, false, &okay);
        if (okay == false) {
            onError(QString("Connection to Serial Port canceled by user."));
            return;
        }
    }

    if (portString.isEmpty() == false) {
        // CREATE A SERIAL PORT OBJECT
        port = new QSerialPort();

        // SET THE SERIAL PORT SETTINGS
        ((QSerialPort *)port)->setPortName(portString);
        ((QSerialPort *)port)->setBaudRate(QSerialPort::Baud38400);
        ((QSerialPort *)port)->setDataBits(QSerialPort::Data8);
        ((QSerialPort *)port)->setStopBits(QSerialPort::OneStop);
        ((QSerialPort *)port)->setParity(QSerialPort::NoParity);
        ((QSerialPort *)port)->setFlowControl(QSerialPort::NoFlowControl);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUTCPSerialPortClient::LAUTCPSerialPortClient(QString ipAddr, int portNum, QObject *parent) : QObject(parent), ipAddress(ipAddr), portNumber(portNum), port(NULL)
{
    if (ipAddress.isEmpty()) {
        // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
        LAUZeroConfClientDialog dialog(QString("_lautcpserialportserver._tcp"));
        if (dialog.exec()) {
            ipAddress = dialog.address();
            portNumber = dialog.port();
        } else {
            return;
        }
    }

    // CREATE A TCP SOCKET OBJECT
    if (ipAddress.isEmpty() == false) {
        port = new QTcpSocket();
        connect(((QTcpSocket *)port), SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(((QTcpSocket *)port), SIGNAL(connected()), this, SLOT(onConnected()));
        connect(((QTcpSocket *)port), SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        connect(((QTcpSocket *)port), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUTCPSerialPortClient::connectPort()
{
    // MAKE SURE WE HAVE WHAT WE NEED FOR A CONNECTION
    if (port) {
        // CREATE A NEW CONNECTION OR OPEN THE SERIAL PORT FOR COMMUNICATION
        if (dynamic_cast<QTcpSocket *>(port) != NULL) {
            ((QTcpSocket *)port)->connectToHost(ipAddress, portNumber, QIODevice::ReadWrite);
            if (((QTcpSocket *)port)->waitForConnected(3000) == false) {
                errorString = QString("Cannot connect to Serial Port.\n") + port->errorString();
                onError(errorString);
            } else if (!port->isReadable()) {
                errorString = QString("Port is not readable!\n") + port->errorString();
                onError(errorString);
            } else {
                connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
                return (true);
            }
        } else {
            if (!port->open(QIODevice::ReadWrite)) {
                errorString = QString("Cannot connect to Serial Port.\n") + port->errorString();
                onError(errorString);
            } else if (!port->isReadable()) {
                errorString = QString("Port is not readable!\n") + port->errorString();
                onError(errorString);
            } else {
                connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
                return (true);
            }
        }
    } else {
        errorString = QString("No valid serial port or IP address.\n");
        onError(errorString);
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUTCPSerialPortClient::~LAUTCPSerialPortClient()
{
    // CLOSE THE SERIAL PORT IF IT IS CURRENTLY OPEN
    if (port && port->isOpen()) {
        port->close();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUTCPSerialPortClient::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            onError(QString("LAU3DVideoTCPClient :: Remote host closed error!"));
            break;
        case QAbstractSocket::HostNotFoundError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            onError(QString("LAU3DVideoTCPClient :: Host not found error!"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            onError(QString("LAU3DVideoTCPClient :: Connection refused error!"));
            break;
        default:
            onError(QString("LAU3DVideoTCPClient :: Default error!"));
            break;
    }
}
#endif
