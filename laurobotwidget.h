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

#ifndef LAUBUTTONWIDGET_H
#define LAUBUTTONWIDGET_H

#ifndef LAU_SERVER
#include <QWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QInputDialog>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <QVector>

#include "laurplidarwidget.h"
#include "laupalettewidget.h"
#endif

#include <QTime>
#include <QList>
#include <QDebug>
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "lautcpserialportwidget.h"

#define LAUROBOT_SERVERIDSTRING   "_lautcprobotserver._tcp"
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
#define LAUROBOT_RESETENCODERVALUES                    20
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
#define LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2  69
#define LAUROBOT_SETS3S4ANDS5MODES                     74
#define LAUROBOT_READS3S4ANDS5MODES                    75
#define LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS        76
#define LAUROBOT_READDEADBANDFORRCANALOGCONTROLS       77
#define LAUROBOT_READENCODERVALUES                     78
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
class LAURobotServer : public LAUTCPSerialPortServer
{
    Q_OBJECT

public:
    explicit LAURobotServer(int num = LAUTCPSERIALPORTSERVERPORTNUMER, unsigned short identifier = 0xFFFF, QObject *parent = 0) : LAUTCPSerialPortServer(num, identifier, QString(LAUROBOT_SERVERIDSTRING), parent) { ; }

};

#ifndef LAU_SERVER
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURobotObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    LAURobotObject(QString portString, QObject *parent) : LAUTCPSerialPortClient(portString, parent), firmwareString(QString("DEMO")) { ; }
    LAURobotObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, QString(LAUROBOT_SERVERIDSTRING), parent), firmwareString(QString("Connected RoboClaw")) { ; }
    ~LAURobotObject();

    QString firmware() const
    {
        return (firmwareString);
    }

public slots:
    void onSendMessage(int message, void *argument = NULL);
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    enum CRC { CRCSend, CRCReceive };

    QList<unsigned short> crcList;
    QList<int> messageIDList;
    QByteArray messageArray;
    QString firmwareString;

    bool processMessage();

    QVector<QPointF> points;
    bool view_has_run;
    void addPoints(float p1, float p2);
    void encoderWidget(QVector<QPointF> points);

private slots:
    void onTcpError(QAbstractSocket::SocketError error);

    QByteArray appendCRC(QByteArray byteArray, CRC state);
    bool checkCRC(QByteArray byteArray, CRC state);

signals:
    void emitError(QString string);
    void emitPoint(QPoint pt);
    void emitMessage(int message, void *argument = NULL);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURobotWidget : public LAUPaletteWidget
{
    Q_OBJECT

public:
    LAURobotWidget(QString portString, QWidget *parent = 0);
    LAURobotWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~LAURobotWidget();

protected:
    void showEvent(QShowEvent *);

public slots:
    void onTCPError(QString string);
    void onReceiveMessage(int message, void *argument = NULL);
    void onRequestEncoder();

    void onValueChanged(QPoint pos, int val);
    void onButtonPressed(QPoint pos);
    void onButtonReleased(QPoint pos);

private:
    LAURobotObject *robot;
    LAURPLidarLabel *plotWidget;

signals:
    void emitMessage(int message, void *argument = NULL);
    void emitPoint(QPoint pt);
};
#endif
#endif // LAUBUTTONWIDGET_H
