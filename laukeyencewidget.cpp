/**************************************************************************************************
    Copyright (C) 2017 Dr. Daniel L. Lau
    This file is part of LAUKeyenceWidget.

    LAUKeyenceWidget is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LAUKeyenceWidget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LAUKeyenceWidget.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************************************/
#include "laukeyencewidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceWidget::LAUKeyenceWidget(QString portString, QWidget *parent) : QWidget(parent)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAUKeyenceWidget");

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new LAUKeyenceObject(portString);
    connect(this, SIGNAL(emitMessage(int, void *)), object, SLOT(onSendMessage(int, void *)));
    connect(object, SIGNAL(emitMessage(int, void *)), this, SLOT(onReceiveMessage(int, void *)));
    connect(object, SIGNAL(emitError(QString)), this, SLOT(onTCPError(QString)));

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        //this->setWindowTitle(object->firmware());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceWidget::LAUKeyenceWidget(QString ipAddr, int portNum, QWidget *parent) : QWidget(parent)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAUKeyenceWidget");

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new LAUKeyenceObject(ipAddr, portNum);
    connect(this, SIGNAL(emitMessage(int, void *)), object, SLOT(onSendMessage(int, void *)));
    connect(object, SIGNAL(emitMessage(int, void *)), this, SLOT(onReceiveMessage(int, void *)));
    connect(object, SIGNAL(emitError(QString)), this, SLOT(onTCPError(QString)));

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        //this->setWindowTitle(object->firmware());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceWidget::~LAUKeyenceWidget()
{
    if (object) {
        delete object;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceWidget::onTCPError(QString string)
{
    QMessageBox::critical(0, QString("Keyence Widget"), string, QMessageBox::Ok);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceWidget::onReceiveMessage(int message, void *argument)
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceWidget::showEvent(QShowEvent *)
{
    // SEND THE INITIAL HANDSHAKE MESSAGES
    emit emitMessage(LAUKEYENCE_READ);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceObject::LAUKeyenceObject(QString portString, QObject *parent) : QObject(parent), ipAddress(portString), portNumber(-1), port(NULL)
{
    if (portString.isEmpty()) {
        // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
        QStringList ports;
        QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
        for (int n = 0; n < portList.count(); n++) {
            ports << portList.at(n).portName();
        }

        // ASK THE USER WHICH PORT SHOULD WE USE AND THEN TRY TO CONNECT
        bool okay = false;
        portString = QInputDialog::getItem(NULL, QString("IL-030"), QString("Select IL-030 Port"), ports, 0, false, &okay);
        if (okay == false) {
            emit emitError(QString("Connection to IL-030 canceled by user."));
        }
    }

    if (portString.isEmpty() == false) {
        // CREATE A SERIAL PORT OBJECT
        port = new QSerialPort();

        // SET THE SERIAL PORT SETTINGS
        ((QSerialPort *)port)->setPortName(portString);
        ((QSerialPort *)port)->setBaudRate(QSerialPort::Baud9600);
        ((QSerialPort *)port)->setDataBits(QSerialPort::Data8);
        ((QSerialPort *)port)->setStopBits(QSerialPort::OneStop);
        ((QSerialPort *)port)->setParity(QSerialPort::NoParity);
        ((QSerialPort *)port)->setFlowControl(QSerialPort::NoFlowControl);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceObject::LAUKeyenceObject(QString ipAddr, int portNum, QObject *parent) : QObject(parent), ipAddress(ipAddr), portNumber(portNum), port(NULL)
{
    if (ipAddress.isEmpty()) {
        // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
        LAUZeroConfClientDialog dialog(QString("_lautcpserialportserver._tcp"));
        if (dialog.exec()) {
            ipAddress = dialog.address();
            portNumber = dialog.port();
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
bool LAUKeyenceObject::connectPort()
{
    // MAKE SURE WE HAVE WHAT WE NEED FOR A CONNECTION
    if (port) {
        // CREATE A NEW CONNECTION OR OPEN THE SERIAL PORT FOR COMMUNICATION
        if (dynamic_cast<QTcpSocket *>(port) != NULL) {
            ((QTcpSocket *)port)->connectToHost(ipAddress, portNumber, QIODevice::ReadWrite);
            if (((QTcpSocket *)port)->waitForConnected(3000) == false) {
                errorString = QString("Cannot connect to IL-030.\n") + port->errorString();
                emit emitError(errorString);
            } else if (!port->isReadable()) {
                errorString = QString("Port is not readable!\n") + port->errorString();
                emit emitError(errorString);
            } else {
                connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
                return (true);
            }
        } else {
            if (!port->open(QIODevice::ReadWrite)) {
                errorString = QString("Cannot connect to IL-030.\n") + port->errorString();
                emit emitError(errorString);
            } else if (!port->isReadable()) {
                errorString = QString("Port is not readable!\n") + port->errorString();
                emit emitError(errorString);
            } else {
                connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
                return (true);
            }
        }
    } else {
        errorString = QString("No valid serial port or IP address.\n");
        emit emitError(errorString);
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUKeyenceObject::~LAUKeyenceObject()
{
    // CLOSE THE SERIAL PORT IF IT IS CURRENTLY OPEN
    if (port && port->isOpen()) {
        port->close();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceObject::onConnected()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceObject::onDisconnected()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceObject::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            emit emitError(QString("LAUKeyenceTCPClient :: Remote host closed error!"));
            break;
        case QAbstractSocket::HostNotFoundError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAUKeyenceTCPClient :: Host not found error!"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAUKeyenceTCPClient :: Connection refused error!"));
            break;
        default:
            emit emitError(QString("LAUKeyenceTCPClient :: Default error!"));
            break;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceObject::onSendMessage(int message, void *argument)
{
    // MAKE SURE WE HAVE AN OPEN PORT BEFORE BUILDING OUR MESSAGE
    if (!port || port->isOpen() == false) {
        return;
    }

    // CREATE A CHARACTER BUFFER TO HOLD THE MESSAGE
    QByteArray byteArray;

    if (message == LAUKEYENCE_READ) {
        byteArray.append((char)0x53);
        byteArray.append((char)0x52);
        byteArray.append((char)0x2c);
        byteArray.append((char)0x30);
        byteArray.append((char)0x32);
        byteArray.append((char)0x2c);
        byteArray.append((char)0x31);
        byteArray.append((char)0x30);
        byteArray.append((char)0x34);
        byteArray.append((char)0x0d);
        byteArray.append((char)0x0a);
        port->write(byteArray);
        messageIDList.append(message);
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUKeyenceObject::onReadyRead()
{
    // KEEP READING FROM THE PORT UNTIL THERE ARE NO MORE BYTES TO READ
    while (port->bytesAvailable() > 0) {
        // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
        QByteArray byteArray = port->readAll();
        if (byteArray.isEmpty() == false) {
            // APPEND INCOMING BYTE ARRAY TO ALL SITTING BYTES
            messageArray.append(byteArray);
            do {
                qDebug() << messageArray;
                qApp->processEvents();
            } while (processMessage());
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUKeyenceObject::processMessage()
{
    if (messageIDList.count() == 0 || messageArray.isEmpty()) {
        return (false);
    }

    // NOW SEE IF WE HAVE A COMPLETE REPLY TO OUR LAST MESSAGE
    int message = messageIDList.first();
    switch (message) {
        case LAUKEYENCE_READ: {
            if (messageArray.length() >= 20) {
                QByteArray byteArray = messageArray.left(20);
                qDebug() << byteArray;
                messageArray.remove(0, 20);
                return (true);
            }
        }
    }
    return (false);
}
