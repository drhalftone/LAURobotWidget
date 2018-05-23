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
#include "laurobotwidget.h"

#ifndef LAU_SERVER
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotWidget::LAURobotWidget(QString ipAddr, int portNum, QWidget *parent) : LAUPaletteWidget(QString("RoboClaw"), QList<LAUPalette::Packet>(), parent), robot(NULL), plotWidget(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAURobotWidget");

    // REGISTER THE PALETTE WIDGETS
    QList<LAUPalette::Packet> packets;
    LAUPalette::Packet packet;
    packet.pal = LAUPaletteObject::PaletteButton;
    packet.pos = QPoint(0, 1);
    packets << packet;
    packet.pal = LAUPaletteObject::PaletteSlider;
    packet.pos = QPoint(1, 0);
    packets << packet;
    packet.pal = LAUPaletteObject::PaletteSlider;
    packet.pos = QPoint(-1, 0);
    packets << packet;

    this->registerLayout(packets);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    robot = new LAURobotObject(ipAddr, portNum, NULL);
    connect(this, SIGNAL(emitMessage(int, void *)), robot, SLOT(onSendMessage(int, void *)));
    connect(robot, SIGNAL(emitMessage(int, void *)), this, SLOT(onReceiveMessage(int, void *)));
    connect(robot, SIGNAL(emitError(QString)), this, SLOT(onTCPError(QString)));

    // CREATE A WIDGE TO DISPLAY ENCODER DATA
    plotWidget = new LAURPLidarLabel();
    connect(robot, SIGNAL(emitPoint(QPoint)), plotWidget, SLOT(onAddPoint(QPoint)));

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (robot->connectPort()) {
        this->setWindowTitle(robot->firmware());

        // DISPLAY THE PLOT WIDGET
        plotWidget->show();
        plotWidget->setMinimumHeight(300);
        plotWidget->setMinimumWidth(300);

        // LET'S START THE BALL ROLLING BY ASKING FOR THE CURRENT ENCODER VALUES
        onRequestEncoder();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotWidget::LAURobotWidget(QString portString, QWidget *parent) : LAUPaletteWidget(QString("RoboClaw"), QList<LAUPalette::Packet>(), parent), robot(NULL), plotWidget(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAURobotWidget");

    // REGISTER THE PALETTE WIDGETS
    QList<LAUPalette::Packet> packets;
    LAUPalette::Packet packet;
    packet.pal = LAUPaletteObject::PaletteButton;
    packet.pos = QPoint(0, 1);
    packets << packet;
    packet.pal = LAUPaletteObject::PaletteSlider;
    packet.pos = QPoint(1, 0);
    packets << packet;
    packet.pal = LAUPaletteObject::PaletteSlider;
    packet.pos = QPoint(-1, 0);
    packets << packet;

    this->registerLayout(packets);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    robot = new LAURobotObject(portString, (QObject*)NULL);
    connect(this, SIGNAL(emitMessage(int, void *)), robot, SLOT(onSendMessage(int, void *)));
    connect(robot, SIGNAL(emitMessage(int, void *)), this, SLOT(onReceiveMessage(int, void *)));
    connect(robot, SIGNAL(emitError(QString)), this, SLOT(onTCPError(QString)));

    // CREATE A WIDGE TO DISPLAY ENCODER DATA
    plotWidget = new LAURPLidarLabel();
    plotWidget->setMinimumSize(480,480);
    connect(robot, SIGNAL(emitPoint(QPoint)), plotWidget, SLOT(onAddPoint(QPoint)));

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (robot->connectPort()) {
        this->setWindowTitle(robot->firmware());

        // DISPLAY THE PLOT WIDGET
        plotWidget->show();

        // LET'S START THE BALL ROLLING BY ASKING FOR THE CURRENT ENCODER VALUES
        onRequestEncoder();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotWidget::~LAURobotWidget()
{
    if (robot) {
        delete robot;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onRequestEncoder()
{
    emit emitMessage(LAUROBOT_READENCODERVALUES);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onValueChanged(QPoint pos, int val)
{
    // DETERMINE IF INCOMING SIGNAL IS FROM LEFT OR RIGHT SLIDER

    // RIGHT SLIDER
    if (pos == QPoint(1, 0)) {
        unsigned char uval = (unsigned char)((255 - val) / 2);
        //Switched to DRIVEMOTOR2 to match Palette and tread orientation -MM//
        emit emitMessage(LAUROBOT_DRIVEMOTOR2_7BIT, &uval);
    }
    // LEFT SLIDER
    else if (pos == QPoint(-1, 0)) {
        unsigned char uval = (unsigned char)((255-val) / 2);
        //Switched to DRIVEMOTOR1 to match Palette and tread orientation -MM//
        emit emitMessage(LAUROBOT_DRIVEMOTOR1_7BIT, &uval);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onButtonPressed(QPoint pos)
{
    // DETERMINE IF SIGNAL IS FROM PUSH BUTTON
    if (pos == QPoint(0, 1)) {

    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// THIS FUNCTION IS THE EMERGENCY STOP BUTTON PRESS -MM//

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onButtonReleased(QPoint pos)
{
    // DETERMINE IF SIGNAL IS FROM PUSH BUTTON
    if (pos == QPoint(0, 1)) {
        //Setting the motor value to 0 so that the motors will stop.//

        unsigned char val = 0;
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
        qDebug() << "SENT: ALL STOP";
        emit emitMessage(LAUROBOT_READENCODERVALUES);
        emit emitMessage(LAUROBOT_RESETENCODERVALUES);
        emit emitMessage(LAUROBOT_READENCODERVALUES);

    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onTCPError(QString string)
{
    QMessageBox::critical(0, QString("Robot Widget"), string, QMessageBox::Ok);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onReceiveMessage(int message, void *argument)
{
    Q_UNUSED(argument);

    // WE RECEIVED AN ENCODER VALUE, SO LET'S ASK AGAIN AFTER A 100 MILLISECOND DELAY
    if (message == LAUROBOT_READENCODERVALUES){
        QTimer::singleShot(100, this, SLOT(onRequestEncoder()));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::showEvent(QShowEvent *)
{
    // SEND THE INITIAL HANDSHAKE MESSAGES
    emit emitMessage(LAUROBOT_READFIRMWAREVERSION);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotObject::~LAURobotObject()
{
    // CLOSE THE SERIAL PORT IF IT IS CURRENTLY OPEN
    if (isValid()) {
        unsigned char val = 0;
        onSendMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        onSendMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QByteArray LAURobotObject::appendCRC(QByteArray byteArray, CRC state)
{
    // IF THIS IS RECEIVED MESSAGE, INITIALIZE THE CRC CODE TO THE NEXT ONE IN THE QUE
    unsigned short crc = 0;
    if (state == CRCReceive) {
        if (crcList.count() > 0) {
            crc = crcList.takeFirst();
        } else {
            return (QByteArray());
        }
    }

    // CALCULATE THE CRC CHECK SUM FOR THE USER SUPPLIED BYTE ARRAY
    for (int byte = 0; byte < byteArray.length(); byte++) {
        crc = crc ^ ((unsigned char)byteArray.at(byte) << 8);
        for (unsigned char bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = (crc << 1);
            }
        }
    }

    // ADD THE CRC TO OUR SENT MESSAGE LIST FOR LATER
    if (state == CRCSend) {
        crcList.append(crc);
    }

    // APPEND THE CRC CHECK SUM IN BIG ENDIAN FORMAT
    byteArray.append((char)((crc >> 8) & 0x00ff));
    byteArray.append((char)((crc >> 0) & 0x00ff));

    return (byteArray);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAURobotObject::checkCRC(QByteArray byteArray, CRC state)
{
    // REMOVE THE CRC BYTES FROM THE BYTE ARRAY AND CALCULATE THE CHECK SUM
    QByteArray localArray = appendCRC(byteArray.left(byteArray.length() - 2), state);

    // MAKE SURE THE NEW CHECK SUM MATCHES THE USER SUPPLIED CHECK SUM
    if (localArray.isEmpty() == false) {
        int index = byteArray.length();
        if (localArray.at(index - 1) == byteArray.at(index - 1)) {
            if (localArray.at(index - 2) == byteArray.at(index - 2)) {
                return (true);
            }
        }
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onConnected()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onDisconnected()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            emit emitError(QString("LAU3DVideoTCPClient :: Remote host closed error!"));
            break;
        case QAbstractSocket::HostNotFoundError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAU3DVideoTCPClient :: Host not found error!"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            // KEEP TRYING UNTIL WE GET A CONNECTION
            emit emitError(QString("LAU3DVideoTCPClient :: Connection refused error!"));
            break;
        default:
            emit emitError(QString("LAU3DVideoTCPClient :: Default error!"));
            break;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onSendMessage(int message, void *argument)
{
    // MAKE SURE WE HAVE AN OPEN PORT BEFORE BUILDING OUR MESSAGE
    if (isValid()) {
        // CREATE A CHARACTER BUFFER TO HOLD THE MESSAGE
        QByteArray byteArray(1, (char)LAUROBOT_WIDGETADDRESS);
        if (message == LAUROBOT_DRIVEFORWARDMOTOR1) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR1);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDMOTOR1";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR1) {
            byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR1);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEBACKWARDSMOTOR1";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEFORWARDMOTOR2) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR2);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDMOTOR2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR2) {
            byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR2);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEBACKWARDSMOTOR2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEMOTOR1_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEMOTOR1_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEMOTOR1_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEMOTOR2_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEMOTOR2_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEMOTOR2_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEFORWARDMIXEDMODE) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDMIXEDMODE);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDMIXEDMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEBACKWARDSMIXEDMODE) {
            byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMIXEDMODE);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEBACKWARDSMIXEDMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_TURNRIGHTMIXEDMODE) {
            byteArray.append((char)LAUROBOT_TURNRIGHTMIXEDMODE);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_TURNRIGHTMIXEDMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_TURNLEFTMIXEDMODE) {
            byteArray.append((char)LAUROBOT_TURNLEFTMIXEDMODE);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_TURNLEFTMIXEDMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_TURNLEFTORRIGHT_7BIT) {
            byteArray.append((char)LAUROBOT_TURNLEFTORRIGHT_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_TURNLEFTORRIGHT_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READFIRMWAREVERSION) {
            byteArray.append((char)LAUROBOT_READFIRMWAREVERSION);
            qDebug() << "SENT: LAUROBOT_READFIRMWAREVERSION";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_RESETENCODERVALUES) {
            byteArray.append((char)LAUROBOT_RESETENCODERVALUES);
            qDebug() << "SENT: LAUROBOT_RESETENCODERVALUES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READMAINBATTERYVOLTAGE) {
            byteArray.append((char)LAUROBOT_READMAINBATTERYVOLTAGE);
            qDebug() << "SENT: LAUROBOT_READMAINBATTERYVOLTAGE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETMINLOGICVOLTAGELEVEL) {
            byteArray.append((char)LAUROBOT_SETMINLOGICVOLTAGELEVEL);
            byteArray.append(0x02); //Setting it to 1. 1=0x01 . Accepted range is 0 to 140 (6V to 34V)
            qDebug() << "SENT: LAUROBOT_SETMINLOGICVOLTAGELEVEL";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETMAXLOGICVOLTAGELEVEL) {
            byteArray.append((char)LAUROBOT_SETMAXLOGICVOLTAGELEVEL);
            byteArray.append(0xaf); //Setting it to 170. 170=0xaa . Accepted range is 30 to 175 (6V to 34V)
            qDebug() << "SENT: LAUROBOT_SETMAXLOGICVOLTAGELEVEL";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READMOTORPWMS) {
            byteArray.append((char)LAUROBOT_READMOTORPWMS);
            qDebug() << "SENT: LAUROBOT_READMOTORPWMS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READMOTORCURRENTS) {
            byteArray.append((char)LAUROBOT_READMOTORCURRENTS);
            qDebug() << "SENT: LAUROBOT_READMOTORCURRENTS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETMAINBATTERYVOLTAGES) {
            byteArray.append((char)LAUROBOT_SETMAINBATTERYVOLTAGES);
            byteArray.append((char)0x00);
            byteArray.append((char)0x46);
            byteArray.append((char)0x01);
            byteArray.append((char)0x2c);
            qDebug() << "SENT: LAUROBOT_SETMAINBATTERYVOLTAGES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETLOGICBATTERYVOLTAGES) {
            byteArray.append((char)LAUROBOT_SETLOGICBATTERYVOLTAGES);
            byteArray.append((char)0x00);
            byteArray.append((char)0x46);
            byteArray.append((char)0x01);
            byteArray.append((char)0x2c);
            qDebug() << "SENT: LAUROBOT_SETLOGICBATTERYVOLTAGES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READMAINBATTERYVOLTAGESETTINGS) {
            byteArray.append((char)LAUROBOT_READMAINBATTERYVOLTAGESETTINGS);
            qDebug() << "SENT: LAUROBOT_READMAINBATTERYVOLTAGESETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS) {
            byteArray.append((char)LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS);
            qDebug() << "SENT: LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1) {
            byteArray.append((char)LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1);
            //Sending in reverse order for Endian-ness of 0x10eb0900
            byteArray.append((char)0x10);
            byteArray.append((char)0xeb);
            byteArray.append((char)0x09);
            byteArray.append((char)0x00);
            qDebug() << "SENT: LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2) {
            byteArray.append((char)LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2);
            //Sending in normal order of 0x0009eb10
            byteArray.append((char)0x00);
            byteArray.append((char)0x09);
            byteArray.append((char)0xeb);
            byteArray.append((char)0x10);
            qDebug() << "SENT: LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETS3S4ANDS5MODES) {
            byteArray.append((char)LAUROBOT_SETS3S4ANDS5MODES);

            char s3 = 0x01;
            char s4 = 0x02;
            char s5 = 0x03;

            byteArray.append(s3); //Setting it to 1. 1=0x01 . 1 is S3 mode default
            byteArray.append(s4); //Setting it to 2. 2=0x02 . 2 is S4 mode regular E-stop
            byteArray.append(s5); //Setting it to 4. 4=0x04 . 4 is S5 mode M2 home
            qDebug() << "SENT: LAUROBOT_SETS3S4ANDS5MODES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READS3S4ANDS5MODES) {
            byteArray.append((char)LAUROBOT_READS3S4ANDS5MODES);
            qDebug() << "SENT: LAUROBOT_READS3S4ANDS5MODES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS) {
            byteArray.append((char)LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS);
            byteArray.append((char)0x1a); //Setting it to 26. 26=0x1a . 25=0x19 is default
            byteArray.append((char)0x1b); //Setting it to 27. 27=0x1b . 25=0x19 is default
            qDebug() << "SENT: LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READDEADBANDFORRCANALOGCONTROLS) {
            byteArray.append((char)LAUROBOT_READDEADBANDFORRCANALOGCONTROLS);
            qDebug() << "SENT: LAUROBOT_READDEADBANDFORRCANALOGCONTROLS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_RESTOREDEFAULTS) {
            byteArray.append((char)LAUROBOT_RESTOREDEFAULTS);
            qDebug() << "SENT: LAUROBOT_RESTOREDEFAULTS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS) {
            byteArray.append((char)LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS);
            qDebug() << "SENT: LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READTEMPERATURE) {
            byteArray.append((char)LAUROBOT_READTEMPERATURE);
            qDebug() << "SENT: LAUROBOT_READTEMPERATURE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READTEMPERATURE2) {
            byteArray.append((char)LAUROBOT_READTEMPERATURE);
            qDebug() << "SENT: LAUROBOT_READTEMPERATURE2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READSTATUS) {
            byteArray.append((char)LAUROBOT_READSTATUS);
            qDebug() << "SENT: LAUROBOT_READSTATUS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READENCODERMODES) {
            byteArray.append((char)LAUROBOT_READENCODERMODES);
            qDebug() << "SENT: LAUROBOT_READENCODERMODES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETMOTOR1ENCODERMODE) {
            byteArray.append((char)LAUROBOT_SETMOTOR1ENCODERMODE);
            byteArray.append((char)0x81);
            qDebug() << "SENT: LAUROBOT_SETMOTOR1ENCODERMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETMOTOR2ENCODERMODE) {
            byteArray.append((char)LAUROBOT_SETMOTOR2ENCODERMODE);
            byteArray.append((char)0x81);
            qDebug() << "SENT: LAUROBOT_SETMOTOR2ENCODERMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//Unknown
//
        else if (message == LAUROBOT_WRITESETTINGSTOEEPROM) {
            byteArray.append((char)LAUROBOT_WRITESETTINGSTOEEPROM);
            qDebug() << "SENT: LAUROBOT_WRITESETTINGSTOEEPROM";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
// ERROR - Must contact ION MC about this issue
//
        else if (message == LAUROBOT_READSETTINGSFROMEEPROM) {
            byteArray.append((char)LAUROBOT_READSETTINGSFROMEEPROM);
            qDebug() << "SENT: LAUROBOT_READSETTINGSFROMEEPROM";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETSTANDARDCONGSETTINGS) {
            byteArray.append((char)LAUROBOT_SETSTANDARDCONGSETTINGS);
            byteArray.append((char)0x40);
            byteArray.append((char)0x00);
            qDebug() << "SENT: LAUROBOT_SETSTANDARDCONGSETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READSTANDARDCONGSETTINGS) {
            byteArray.append((char)LAUROBOT_READSTANDARDCONGSETTINGS);
            qDebug() << "SENT: LAUROBOT_READSTANDARDCONGSETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETM1MAXIMUMCURRENT) {
            byteArray.append((char)LAUROBOT_SETM1MAXIMUMCURRENT);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x0f);
            byteArray.append((char)0xa0);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            qDebug() << "SENT: LAUROBOT_SETM1MAXIMUMCURRENT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETM2MAXIMUMCURRENT) {
            byteArray.append((char)LAUROBOT_SETM2MAXIMUMCURRENT);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x0f);
            byteArray.append((char)0xa0);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            qDebug() << "SENT: LAUROBOT_SETM2MAXIMUMCURRENT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READM1MAXIMUMCURRENT) {
            byteArray.append((char)LAUROBOT_READM1MAXIMUMCURRENT);
            qDebug() << "SENT: LAUROBOT_READM1MAXIMUMCURRENT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READM2MAXIMUMCURRENT) {
            byteArray.append((char)LAUROBOT_READM2MAXIMUMCURRENT);
            qDebug() << "SENT: LAUROBOT_READM2MAXIMUMCURRENT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READENCODERVALUES) {
            byteArray.append((char)LAUROBOT_READENCODERVALUES);
            qDebug() << "SENT: LAUROBOT_READENCODERVALUES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_SETPWMMODE) {
            byteArray.append((char)LAUROBOT_SETPWMMODE);
            byteArray.append((char)0x01);
            qDebug() << "SENT: LAUROBOT_SETPWMMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
        else if (message == LAUROBOT_READPWMMODE) {
            byteArray.append((char)LAUROBOT_READPWMMODE);
            qDebug() << "SENT: LAUROBOT_READPWMMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onReadyRead()
{
    // KEEP READING FROM THE PORT UNTIL THERE ARE NO MORE BYTES TO READ
    while (bytesAvailable() > 0) {
        // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
        QByteArray byteArray = readAll();
        if (byteArray.isEmpty() == false) {
            // APPEND INCOMING BYTE ARRAY TO ALL SITTING BYTES
            messageArray.append(byteArray);
            do {
                qDebug() << "received: " << messageArray;
                qApp->processEvents();
            } while (processMessage());
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAURobotObject::processMessage()
{
    if (messageIDList.count() == 0 || messageArray.isEmpty()) {
        return (false);
    }

    // NOW SEE IF WE HAVE A COMPLETE REPLY TO OUR LAST MESSAGE
    int message = messageIDList.first();
    switch (message) {
        case LAUROBOT_READFIRMWAREVERSION: {
            int index = messageArray.indexOf('\n');
            if (index != -1 && messageArray.length() >= index + 4) {
                if (checkCRC(messageArray.left(index+4), CRCReceive)) {
                    firmwareString = QString(messageArray.left(index));
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READFIRMWAREVERSION message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, index + 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READMAINBATTERYVOLTAGE: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)) {
                    float volt = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 10.0;
                    qDebug() << "LAUROBOT_READMAINBATTERYVOLTAGE: " << volt <<"v";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READMAINBATTERYVOLTAGE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READTEMPERATURE: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    float temp = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 10.0;
                    qDebug() << "LAUROBOT_READTEMPERATURE: " << temp << " C";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READTEMPERATURE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READTEMPERATURE2: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    float temp = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 10.0;
                    qDebug() << "LAUROBOT_READTEMPERATURE2: " << temp << " C";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READTEMPERATURE2 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READMOTORPWMS: {
            if (messageArray.length() >= 6) {
                if (checkCRC(messageArray.left(6), CRCReceive)){
                    float m1pwm = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 327.67;
                    float m2pwm = ((uint8_t)messageArray.at(2) << 8 | (uint8_t)messageArray.at(3)) / 327.67;
                    qDebug() << "LAUROBOT_READMOTORPWMS: ";
                    qDebug() << "Motor 1 PWMS: "  << m1pwm;
                    qDebug() << "Motor 2 PWMS: "  << m2pwm;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READMOTORPWMS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 6);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READMOTORCURRENTS: {
            if (messageArray.length() >= 6) {
                if (checkCRC(messageArray.left(6), CRCReceive)){
                    float m1curr = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 100.0;
                    float m2curr = ((uint8_t)messageArray.at(2) << 8 | (uint8_t)messageArray.at(3)) / 100.0;
                    qDebug() << "LAUROBOT_READMOTORCURRENTS: ";
                    qDebug() << "Motor 1 Current: "  << m1curr << " mA";
                    qDebug() << "Motor 2 Current: "  << m2curr << " mA";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READMOTORCURRENTS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 6);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READMAINBATTERYVOLTAGESETTINGS: {
            if (messageArray.length() >= 6) {
                if (checkCRC(messageArray.left(6), CRCReceive)){
                    float vmin = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 10.0;
                    float vmax = ((uint8_t)messageArray.at(2) << 8 | (uint8_t)messageArray.at(3)) / 10.0;
                    qDebug() << "LAUROBOT_READMAINBATTERYVOLTAGESETTINGS: ";
                    qDebug() << "Min: "  << vmin << " V";
                    qDebug() << "Max: "  << vmax << " V";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READMAINBATTERYVOLTAGESETTINGS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 6);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS: {
            if (messageArray.length() >= 6) {
                if (checkCRC(messageArray.left(6), CRCReceive)){
                    float vmin = ((uint8_t)messageArray.at(0) << 8 | (uint8_t)messageArray.at(1)) / 10.0;
                    float vmax = ((uint8_t)messageArray.at(2) << 8 | (uint8_t)messageArray.at(3)) / 10.0;
                    qDebug() << "LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS: ";
                    qDebug() << "Min: "  << vmin << " V";
                    qDebug() << "Max: "  << vmax << " V";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 6);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READS3S4ANDS5MODES: {
            if (messageArray.length() >= 5) {
                if (checkCRC(messageArray.left(5), CRCReceive)){
                    uint8_t s3 = ((uint8_t)messageArray.at(0));
                    uint8_t s4 = ((uint8_t)messageArray.at(1));
                    uint8_t s5 = ((uint8_t)messageArray.at(2));
                    qDebug() << "LAUROBOT_READS3S4ANDS5MODES: ";
                    qDebug() << "s3: "  << s3;
                    qDebug() << "s4: "  << s4;
                    qDebug() << "s5: "  << s5;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READS3S4ANDS5MODES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 5);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READDEADBANDFORRCANALOGCONTROLS: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    float rev = ((uint8_t)messageArray.at(0)) / 10.0;
                    float sfor = ((uint8_t)messageArray.at(1)) / 10.0;
                    qDebug() << "LAUROBOT_READDEADBANDFORRCANALOGCONTROLS: ";
                    qDebug() << "Reverse: "  << rev << " %";
                    qDebug() << "Forward: "  << sfor << " %";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READDEADBANDFORRCANALOGCONTROLS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS: {
            if (messageArray.length() >= 18) {
                if (checkCRC(messageArray.left(18), CRCReceive)){
                    int M1A, M1D, M2A, M2D;

                    ((char*)&M1A)[0] = (uint8_t)messageArray.at(3);
                    ((char*)&M1A)[1] = (uint8_t)messageArray.at(2);
                    ((char*)&M1A)[2] = (uint8_t)messageArray.at(1);
                    ((char*)&M1A)[3] = (uint8_t)messageArray.at(0);

                    ((char*)&M1D)[0] = (uint8_t)messageArray.at(7);
                    ((char*)&M1D)[1] = (uint8_t)messageArray.at(6);
                    ((char*)&M1D)[2] = (uint8_t)messageArray.at(5);
                    ((char*)&M1D)[3] = (uint8_t)messageArray.at(4);

                    ((char*)&M2A)[0] = (uint8_t)messageArray.at(11);
                    ((char*)&M2A)[1] = (uint8_t)messageArray.at(10);
                    ((char*)&M2A)[2] = (uint8_t)messageArray.at(9);
                    ((char*)&M2A)[3] = (uint8_t)messageArray.at(8);

                    ((char*)&M2D)[0] = (uint8_t)messageArray.at(15);
                    ((char*)&M2D)[1] = (uint8_t)messageArray.at(14);
                    ((char*)&M2D)[2] = (uint8_t)messageArray.at(13);
                    ((char*)&M2D)[3] = (uint8_t)messageArray.at(12);

                    qDebug() << "LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS: ";
                    qDebug() << "M1 Duty Cycle Accleration: "  << M1A;
                    qDebug() << "M1 Duty Cycle Decleration: "  << M1D;
                    qDebug() << "M2 Duty Cycle Accleration: "  << M2A;
                    qDebug() << "M2 Duty Cycle Decleration: "  << M2D;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 18);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READSTATUS: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    uint8_t stat1 = ((uint8_t)messageArray.at(0));
                    uint8_t stat2 = ((uint8_t)messageArray.at(1));
                    qDebug() << "LAUROBOT_READSTATUS: ";
                    qDebug() << "Status: 0x"  << stat1 << stat2;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READSTATUS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READSTANDARDCONGSETTINGS: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    qDebug() << "LAUROBOT_READSTANDARDCONGSETTINGS: 0x" << ((uint8_t)messageArray.at(0)) << ((uint8_t)messageArray.at(1));
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READSTANDARDCONGSETTINGS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READENCODERMODES: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    uint8_t m1var = messageArray.at(0);
                    uint8_t m2var = messageArray.at(1);
                    uint8_t var1;
                    uint8_t var2;
                    uint8_t var3;
                    uint8_t var4;
                    var1 = m1var & 0x80;
                    var2 = m1var & 0x01;
                    var3 = m2var & 0x80;
                    var4 = m2var & 0x01;
                    var1 = var1/128;
                    var3 = var3/128;

                    qDebug() << "LAUROBOT_READENCODERMODES";
                    qDebug() << "Motor 1 RC/Analog Support (0 = disabled, 1 = enabled):" << var1;
                    qDebug() << "Motor 1 Encoder Mode (0 = Quadrature, 1 = Absolute):" << var2;
                    qDebug() << "Motor 2 RC/Analog Support (0 = disabled, 1 = enabled):" << var3;
                    qDebug() << "Motor 2 Encoder Mode (0 = Quadrature, 1 = Absolute):" << var4;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READENCODERMODES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        //ASK
        case LAUROBOT_READSETTINGSFROMEEPROM: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    uint8_t enc1 = ((uint8_t)messageArray.at(0));
                    uint8_t enc2 = ((uint8_t)messageArray.at(1));
                    qDebug() << "LAUROBOT_READSETTINGSFROMEEPROM: ";
                    qDebug() << "Control 1: " << enc1;
                    qDebug() << "Control 2: " << enc2;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READSETTINGSFROMEEPROM message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READM1MAXIMUMCURRENT: {
            if (messageArray.length() >= 10) {
                if (checkCRC(messageArray.left(10), CRCReceive)){
                    int Max, Min;

                    ((char*)&Max)[0] = (uint8_t)messageArray.at(3);
                    ((char*)&Max)[1] = (uint8_t)messageArray.at(2);
                    ((char*)&Max)[2] = (uint8_t)messageArray.at(1);
                    ((char*)&Max)[3] = (uint8_t)messageArray.at(0);

                    ((char*)&Min)[0] = (uint8_t)messageArray.at(7);
                    ((char*)&Min)[1] = (uint8_t)messageArray.at(6);
                    ((char*)&Min)[2] = (uint8_t)messageArray.at(5);
                    ((char*)&Min)[3] = (uint8_t)messageArray.at(4);

                    qDebug() << "LAUROBOT_READM1MAXIMUMCURRENT: ";
                    qDebug() << "M1 Max Current: "  << Max/100.0 << " mA";
                    qDebug() << "M1 Min Current: "  << Min/100.0 << " mA";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READM1MAXIMUMCURRENT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 10);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READENCODERVALUES: {
            if (messageArray.length() >= 10) {
                if (checkCRC(messageArray.left(10), CRCReceive)){

                    int M1, M2;

                    ((char*)&M1)[0] = (uint8_t)messageArray.at(3);
                    ((char*)&M1)[1] = (uint8_t)messageArray.at(2);
                    ((char*)&M1)[2] = (uint8_t)messageArray.at(1);
                    ((char*)&M1)[3] = (uint8_t)messageArray.at(0);

                    ((char*)&M2)[0] = (uint8_t)messageArray.at(7);
                    ((char*)&M2)[1] = (uint8_t)messageArray.at(6);
                    ((char*)&M2)[2] = (uint8_t)messageArray.at(5);
                    ((char*)&M2)[3] = (uint8_t)messageArray.at(4);

                    qDebug() << "LAUROBOT_READENCODERVALUES: ";
                    qDebug() << "M1 Encoder Value: "  << -M1 << " counts";
                    qDebug() << "M2 Encoder Value: "  << -M2 << " counts";

                    emit emitPoint(QPoint(-M1, -M2));
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READENCODERVALUES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 10);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READM2MAXIMUMCURRENT: {
            if (messageArray.length() >= 10) {
                if (checkCRC(messageArray.left(10), CRCReceive)){
                    int Max, Min;

                    ((char*)&Max)[0] = (uint8_t)messageArray.at(3);
                    ((char*)&Max)[1] = (uint8_t)messageArray.at(2);
                    ((char*)&Max)[2] = (uint8_t)messageArray.at(1);
                    ((char*)&Max)[3] = (uint8_t)messageArray.at(0);

                    ((char*)&Min)[0] = (uint8_t)messageArray.at(7);
                    ((char*)&Min)[1] = (uint8_t)messageArray.at(6);
                    ((char*)&Min)[2] = (uint8_t)messageArray.at(5);
                    ((char*)&Min)[3] = (uint8_t)messageArray.at(4);

                    qDebug() << "LAUROBOT_READM2MAXIMUMCURRENT: ";
                    qDebug() << "M2 Max Current: "  << Max/100.0 << " mA";
                    qDebug() << "M2 Min Current: "  << Min/100.0 << " mA";
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READM2MAXIMUMCURRENT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 10);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READPWMMODE: {
            if (messageArray.length() >= 3) {
                if (checkCRC(messageArray.left(3), CRCReceive)){
                    uint8_t pwmMode = ((uint8_t)messageArray.at(0));
                    qDebug() << "LAUROBOT_READPWMMODE: " << pwmMode;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READPWMMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 3);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETMAINBATTERYVOLTAGES: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Main Battery Voltages Set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETMAINBATTERYVOLTAGES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_RESTOREDEFAULTS: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Default Settings Restored";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_RESTOREDEFAULTS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_RESETENCODERVALUES: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Motor Encoders Reset";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_RESETENCODERVALUES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETMINLOGICVOLTAGELEVEL: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Min Logic Voltage Level set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETMINLOGICVOLTAGELEVEL message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETMAXLOGICVOLTAGELEVEL: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Max Logic Voltage Level set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETMAXLOGICVOLTAGELEVEL message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETLOGICBATTERYVOLTAGES: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Logic Battery Voltages set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETLOGICBATTERYVOLTAGES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Default Duty Cycle Accleration FORM1 set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Default Duty Cycle Accleration FORM2 set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETS3S4ANDS5MODES: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "S3, S4, and S5 Modes set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETS3S4ANDS5MODES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "DEAD BAND for RC ANALOG Control set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETMOTOR1ENCODERMODE: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "M1 Encoder Mode set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETMOTOR1ENCODERMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETMOTOR2ENCODERMODE: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "M2 Encoder Mode set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETMOTOR2ENCODERMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        //DOUBLE CHECK
        case LAUROBOT_WRITESETTINGSTOEEPROM: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Settings Written to EEPROM";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_WRITESETTINGSTOEEPROM message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETSTANDARDCONGSETTINGS: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "Standard Config settings set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETSTANDARDCONGSETTINGS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETM1MAXIMUMCURRENT: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "M1 Max Current set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETM1MAXIMUMCURRENT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETM2MAXIMUMCURRENT: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "M2 Max Current set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETM2MAXIMUMCURRENT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_SETPWMMODE: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    qDebug() << "PWM mode set";
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_SETPWMMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEFORWARDMOTOR1:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEFORWARDMOTOR1 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEFORWARDMOTOR2:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEFORWARDMOTOR2 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEBACKWARDSMOTOR1:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEBACKWARDSMOTOR1 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEBACKWARDSMOTOR2:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEBACKWARDSMOTOR2 message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEFORWARDMIXEDMODE:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEFORWARDMIXEDMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEBACKWARDSMIXEDMODE:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEBACKWARDSMIXEDMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_TURNRIGHTMIXEDMODE:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_TURNRIGHTMIXEDMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_TURNLEFTMIXEDMODE:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_TURNLEFTMIXEDMODE message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_TURNLEFTORRIGHT_7BIT:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_TURNLEFTORRIGHT_7BIT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEMOTOR1_7BIT:{
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEMOTOR1_7BIT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEMOTOR2_7BIT: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    // REMOVE THE CRC FROM THE SENT MESSAGE SINCE WE AREN'T CHECKING IT HERE
                    crcList.takeFirst();
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR sending LAUROBOT_DRIVEMOTOR2_7BIT message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 1);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
    }
    return (false);
}
#endif
