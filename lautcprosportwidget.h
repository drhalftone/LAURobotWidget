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
#ifndef LAUTCPROSPORTWIDGET_H
#define LAUTCPROSPORTWIDGET_H

#include <QDebug>

#ifdef LAU_CLIENT
#include <QInputDialog>

#include "lauzeroconfwidget.h"
#else
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimerEvent>
#include <qzeroconf.h>
#ifdef LAU_ROS
#include <ros/ros.h>
#include <ros/master.h>
#include <nav_msgs/Odometry.h>
#endif
#endif

#define LAUTCPROSPORTSERVERPORTNUMER  11444
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPROSPort : public QTcpServer
{
    Q_OBJECT

public:
    explicit LAUTCPROSPort(QString string, int prtNmbr = LAUTCPROSPORTSERVERPORTNUMER, QObject *parent = 0);
    ~LAUTCPROSPort();

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

    bool isValid() const
    {
#ifdef LAU_ROS
        return (node.ok());
#else
        return (false);
#endif
    }

    bool isNull() const
    {
        return (!isValid());
    }

    QString topic() const
    {
        return (topicString);
    }

protected:
    void incomingConnection(qintptr handle);

private slots:
    void onDisconnected();
    void onReadyReadTCP();
    void onReadyReadROS();
    void onServicePublished();
    void onServiceError(QZeroConf::error_t error);
    void onTcpError(QAbstractSocket::SocketError error);

private:
    bool connected;            // FLAG TO INDICATE WE ARE CONNECTED TO A CLIENT
    int portNumber;            // PORT NUMBER
    QTcpSocket *socket;        // TCP SOCKET TO HOLD THE INCOMING CONNECTION
    QZeroConf *zeroConf;       // ZERO CONF TO ADVERTISE PORT
    QString clientIPAddress;   // IP ADDRESS OF CLIENT, IF THERE IS ONE

    QString topicString;       // TOPIC STRING THAT WE ARE LISTENING TO
#ifdef LAU_ROS
    ros::NodeHandle node;      // HANDLE TO ROS NODE INSTANCE
    ros::Subscriber subscriber;// HANDLE TO ROS SUBSCRIBER INSTANCE
#endif

signals:
    void emitError(QString string);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPROSPortServer : public QObject
{
    Q_OBJECT

public:
    explicit LAUTCPROSPortServer(int num = LAUTCPROSPORTSERVERPORTNUMER, QString tpc = QString(), QObject *parent = 0);
    ~LAUTCPROSPortServer();

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

protected:
#ifdef LAU_ROS
    void timerEvent(QTimerEvent *)
    {
        ros::spinOnce();
    }
#endif

private:
    QList<LAUTCPROSPort *> ports;
};

#ifdef LAU_CLIENT
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPROSPortClient : public QObject
{
    Q_OBJECT

public:
    LAUTCPROSPortClient(QString portString, QObject *parent = 0);
    LAUTCPROSPortClient(QString ipAddr, int portNum, QObject *parent = 0);
    ~LAUTCPROSPortClient();

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

    void setError(QString string)
    {
        errorString = string;
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
        qDebug() << "LAUTCPROSPortClient ::" << error;
    }

private:
    QString ipAddress;
    int portNumber;
    QIODevice *port;
    QString errorString;

private slots:
    void onTcpError(QAbstractSocket::SocketError error);
};
#endif
#endif // LAUTCPROSPORTWIDGET_H
