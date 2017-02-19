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
    explicit LAUTCPSerialPortServer(int num = LAUTCPSERIALPORTSERVERPORTNUMER, QObject *parent = 0);
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
