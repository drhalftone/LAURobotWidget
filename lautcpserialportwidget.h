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

#endif // LAUTCPSERIALPORTWIDGET_H
