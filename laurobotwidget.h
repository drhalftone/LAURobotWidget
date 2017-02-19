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

#ifndef LAUBUTTONWIDGET_H
#define LAUBUTTONWIDGET_H

#include <QList>
#include <QDebug>
#include <QWidget>
#include <QThread>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSerialPort>
#include <QApplication>
#include <QInputDialog>
#include <QSerialPortInfo>

#include "lauzeroconfwidget.h"

#define LAUROBOT_WIDGETADDRESS                        128
#define LAUROBOT_NULLMESSAGESENT                       -1
#define LAUROBOT_DRIVEFORWARDMOTOR1                     0
#define LAUROBOT_DRIVEBACKWARDSMOTOR1                   1
#define LAUROBOT_SETMAINVOLTAGEMINIMUM                  2
#define LAUROBOT_SETMAINVOLTAGEMAXIMUM                  3
#define LAUROBOT_DRIVEFORWARDMOTOR2                     4
#define LAUROBOT_DRIVEBACKWARDSMOTOR2                   5
#define LAUROBOT_DRIVEMOTOR1_7BIT                       6
#define LAUROBOT_DRIVEMOTOR2_7BIT                       7
#define LAUROBOT_DRIVEFORWARDMIXEDMODE                  8
#define LAUROBOT_DRIVEBACKWARDSMIXEDMODE                9
#define LAUROBOT_TURNRIGHTMIXEDMODE                    10
#define LAUROBOT_TURNLEFTMIXEDMODE                     11
#define LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT           12
#define LAUROBOT_TURNLEFTORRIGHT_7BIT                  13
#define LAUROBOT_READFIRMWAREVERSION                   21
#define LAUROBOT_READMAINBATTERYVOLTAGE                24
#define LAUROBOT_SETMINLOGICVOLTAGELEVEL               26
#define LAUROBOT_SETMAXLOGICVOLTAGELEVEL               27
#define LAUROBOT_READMOTORPWMS                         48
#define LAUROBOT_READMOTORCURRENTS                     49
#define LAUROBOT_SETMAINBATTERYVOLTAGES                57
#define LAUROBOT_SETLOGICBATTERYVOLTAGES               58
#define LAUROBOT_READMAINBATTERYVOLTAGESETTINGS        59
#define LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS       60
#define LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1  68
#define LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2  68
#define LAUROBOT_SETS3S4ANDS5MODES                     74
#define LAUROBOT_READS3S4ANDS5MODES                    75
#define LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS        76
#define LAUROBOT_READDEADBANDFORRCANALOGCONTROLS       77
#define LAUROBOT_RESTOREDEFAULTS                       80
#define LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS     81
#define LAUROBOT_READTEMPERATURE                       82
#define LAUROBOT_READTEMPERATURE2                      83
#define LAUROBOT_READSTATUS                            90
#define LAUROBOT_READENCODERMODES                      91
#define LAUROBOT_SETMOTOR1ENCODERMODE                  92
#define LAUROBOT_SETMOTOR2ENCODERMODE                  93
#define LAUROBOT_WRITESETTINGSTOEEPROM                 94
#define LAUROBOT_READSETTINGSFROMEEPROM                95
#define LAUROBOT_SETSTANDARDCONGSETTINGS               98
#define LAUROBOT_READSTANDARDCONGSETTINGS              99
#define LAUROBOT_SETCTRLMODES                         100
#define LAUROBOT_READCTRLMODES                        101
#define LAUROBOT_SETCTRL1                             102
#define LAUROBOT_SETCTRL2                             103
#define LAUROBOT_READCTRLS                            104
#define LAUROBOT_SETM1MAXIMUMCURRENT                  133
#define LAUROBOT_SETM2MAXIMUMCURRENT                  134
#define LAUROBOT_READM1MAXIMUMCURRENT                 135
#define LAUROBOT_READM2MAXIMUMCURRENT                 136
#define LAUROBOT_SETPWMMODE                           148
#define LAUROBOT_READPWMMODE                          149

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURobotObject : public QObject
{
    Q_OBJECT

public:
    LAURobotObject(QString portString, QObject *parent = 0);
    LAURobotObject(QString ipAddress, int portNumber, QObject *parent = 0);
    ~LAURobotObject();

    bool isValid() const
    {
        if (port) {
            return (port->isOpen());
        }
        return (false);
    }

    QString firmware() const
    {
        return (firmwareString);
    }

    QString error() const
    {
        return (errorString);
    }

public slots:
    void onSendMessage(int message, void *argument = NULL);

private:
    enum CRC { CRCSend, CRCReceive };

    QIODevice *port;
    QString errorString;
    QList<unsigned short> crcList;
    QList<int> messageIDList;
    QByteArray messageArray;

    QString firmwareString;

    bool processMessage();

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);

    QByteArray appendCRC(QByteArray byteArray, CRC state);
    bool checkCRC(QByteArray byteArray, CRC state);

signals:
    void emitError(QString string);
    void emitMessage(int message, void *argument = NULL);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURobotWidget : public QWidget
{
    Q_OBJECT

public:
    LAURobotWidget(QWidget *parent = 0);
    ~LAURobotWidget();

protected:
    void showEvent(QShowEvent *);

public slots:
    void onPushButton_clicked();
    void onError(QString string);
    void onReceiveMessage(int message, void *argument = NULL);

private:
    QList<QPushButton *> buttons;
    LAURobotObject *robot;

signals:
    void emitMessage(int message, void *argument = NULL);
};

#endif // LAUBUTTONWIDGET_H
