/**************************************************************************************************
    Copyright 2017 Dr. Daniel L. Lau

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
**************************************************************************************************/
#include "lautcpserialportwidget.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPortServer::LAUTCPSerialPortServer(int num, QObject *parent) : QObject(parent)
{
    // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (int n = 0; n < portList.count(); n++) {
        ports << new LAUTCPSerialPort(portList.at(n).portName(), num + n);
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
LAUTCPSerialPort::LAUTCPSerialPort(QString string, int prtNmbr, QObject *parent) : QTcpServer(parent), connected(false), portNumber(prtNmbr), portString(string), port(NULL), socket(NULL), zeroConf(NULL)
{
    // SET THE SERIAL PORT SETTINGS
    port.setPortName(portString);
    port.setBaudRate(QSerialPort::Baud115200);
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
    zeroConf->startServicePublish(portString.toUtf8(), "_lautcpserialportserver._tcp", "local", portNumber);
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
        emit emitError(QString("Cannot connect to RoboClaw.\n") + port.errorString());
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
