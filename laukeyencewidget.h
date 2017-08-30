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
#ifndef LAUKEYENCEWIDGET_H
#define LAUKEYENCEWIDGET_H

#include <QList>
#include <QDebug>
#include <QThread>
#include <QSerialPort>
#include <QAbstractSocket>
#include <QSerialPortInfo>

#ifdef LAU_CLIENT
#include <QWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QInputDialog>
#endif

#include "lauzeroconfwidget.h"

#define LAUKEYENCE_READ            0x5352

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUKeyenceObject : public QObject
{
    Q_OBJECT

public:
    LAUKeyenceObject(QString portString, QObject *parent = 0);
    LAUKeyenceObject(QString ipAddr, int portNum, QObject *parent = 0);
    ~LAUKeyenceObject();

    bool connectPort();
    bool isValid() const
    {
        if (port) {
            return (port->isOpen());
        }
        return (false);
    }
    QString error() const
    {
        return (errorString);
    }

public slots:
    void onSendMessage(int message, void *argument = NULL);

private:
    QString ipAddress;
    int portNumber;
    QIODevice *port;
    QString errorString;
    QList<int> messageIDList;
    QByteArray messageArray;

    bool processMessage();

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);

signals:
    void emitError(QString string);
    void emitMessage(int message, void *argument = NULL);
};

#ifdef LAU_CLIENT
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUKeyenceWidget : public QWidget
{
    Q_OBJECT

public:
    LAUKeyenceWidget(QString portString, QWidget *parent = 0);
    LAUKeyenceWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~LAUKeyenceWidget();

protected:
    void showEvent(QShowEvent *);

public slots:
    void onTCPError(QString string);
    void onReceiveMessage(int message, void *argument = NULL);

private:
    LAUKeyenceObject *object;

signals:
    void emitMessage(int message, void *argument = NULL);
};
#endif
#endif // LAUKEYENCEWIDGET_H
