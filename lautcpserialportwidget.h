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

#ifndef LAUTCPSERIALPORTWIDGET_H
#define LAUTCPSERIALPORTWIDGET_H

#include <QInputDialog>

#include "lauzeroconfwidget.h"

#define LAUTCPSERIALPORTSERVERPORTNUMER  11364

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPSerialPort : public QTcpServer
{
    Q_OBJECT

public:
    explicit LAUTCPSerialPort(QString string, int prtNmbr = LAUTCPSERIALPORTSERVERPORTNUMER, QObject *parent = 0);
    ~LAUTCPSerialPort();

    bool isConnected() const
    {
        return (connected);
    }
    QString ipAddress() const
    {
        return (clientIPAddress);
    }
    int localPort() const
    {
        return (portNumber);
    }

protected:
    void incomingConnection(qintptr handle);

private slots:
    void onDisconnected();
    void onReadyReadTCP();
    void onReadyReadSerial();
    void onServicePublished();
    void onServiceError(QZeroConf::error_t error);
    void onTcpError(QAbstractSocket::SocketError error);

private:
    bool connected;            // FLAG TO INDICATE WE ARE CONNECTED TO A CLIENT
    int portNumber;            // PORT NUMBER
    QString portString;        // PORT STRING
    QSerialPort port;          // INSTANCE OF THE SERIAL PORT
    QTcpSocket *socket;        // TCP SOCKET TO HOLD THE INCOMING CONNECTION
    QZeroConf *zeroConf;       // ZERO CONF TO ADVERTISE PORT
    QString clientIPAddress;   // IP ADDRESS OF CLIENT, IF THERE IS ONE

signals:
    void emitError(QString string);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPSerialPortServer : public QObject
{
    Q_OBJECT

public:
    explicit LAUTCPSerialPortServer(int num = LAUTCPSERIALPORTSERVERPORTNUMER, unsigned short identifier = 0xFFFF, QObject *parent = 0);
    ~LAUTCPSerialPortServer();

    int channels() const
    {
        return (ports.count());
    }

    bool isConnected(int n) const
    {
        return (ports.at(n)->isConnected());
    }

    QString ipAddress(int n) const
    {
        return (ports.at(n)->ipAddress());
    }

    int localPort(int n) const
    {
        return (ports.at(n)->localPort());
    }

private:
    QList<LAUTCPSerialPort *> ports;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPSerialPortClient : public QObject
{
    Q_OBJECT

public:
    LAUTCPSerialPortClient(QString portString, QObject *parent = 0);
    LAUTCPSerialPortClient(QString ipAddr, int portNum, QObject *parent = 0);
    ~LAUTCPSerialPortClient();

    bool connectPort();
    bool isValid() const
    {
        return (port && port->isOpen());
    }

    void write(QByteArray byteArray)
    {
        if (isValid()) {
            port->write(byteArray);
        }
    }

    QString error() const
    {
        return (errorString);
    }

    QString address() const
    {
        return (ipAddress);
    }

    int number() const
    {
        return (portNumber);
    }

    void waitForBytesWritten(int msecs = 30000)
    {
        port->waitForBytesWritten(msecs);
    }

    bool bytesAvailable()
    {
        return (port->bytesAvailable());
    }

    QByteArray readAll()
    {
        return (port->readAll());
    }

public slots:
    virtual void onSendMessage(int message, void *argument = NULL) = 0;
    virtual void onReadyRead() = 0;
    virtual void onConnected() { ; }
    virtual void onDisconnected() { ; }
    virtual void onError(QString error)
    {
        qDebug() << "LAUTCPSerialPortClient ::" << error;
    }

private:
    QString ipAddress;
    int portNumber;
    QIODevice *port;
    QString errorString;

private slots:
    void onTcpError(QAbstractSocket::SocketError error);
};

#endif // LAUTCPSERIALPORTWIDGET_H
