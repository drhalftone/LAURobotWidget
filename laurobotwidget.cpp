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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotWidget::LAURobotWidget(QString ipAddr, int portNum, QWidget *parent) : LAUPaletteWidget(QString("RoboClaw"), QList<LAUPalette::Packet>(), parent), robot(NULL)
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
    //robot = new LAURobotObject(ipAddr, portNum, NULL);
    robot = new LAURobotObject(QString(), (QObject *)NULL);
    connect(this, SIGNAL(emitMessage(int, void *)), robot, SLOT(onSendMessage(int, void *)));
    connect(robot, SIGNAL(emitMessage(int, void *)), this, SLOT(onReceiveMessage(int, void *)));
    connect(robot, SIGNAL(emitError(QString)), this, SLOT(onTCPError(QString)));

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (robot->connectPort()) {
        this->setWindowTitle(robot->firmware());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotWidget::LAURobotWidget(QString portString, QWidget *parent) : LAUPaletteWidget(QString("RoboClaw"), QList<LAUPalette::Packet>(), parent), robot(NULL)
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

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (robot->connectPort()) {
        this->setWindowTitle(robot->firmware());
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

// THIS FUNCTION DRIVES THE MOTORS BASED ON SIGNAL FOR LEFT AND RIGHT SLIDERS -MM//

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/* NOTES: -MM
 *
 * Pairing
 *  for LAUROBOT_DRIVEMOTOR2_7BIT
 *      unsigned char uval = (unsigned char)((255 - val) / 2);
 * with
 *  for LAUROBOT_DRIVEMOTOR1_7BIT
 *      unsigned char uval = (unsigned char)((val) / 2);
 * will cause treads to move in opposite directions.
 *
 * MUST  use one or the other to get them to go in the same direction.
 *
 * USE   unsigned char uval = (unsigned char)((255 - val) / 2); for going forward in same direction.
 *
 */


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
        emit emitMessage(LAUROBOT_READMAINBATTERYVOLTAGE);
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

        //NOTE: Ask Dr. LAU aboutthe CRC for the Drive Forward Commands.

        unsigned char val = 0;
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
        qDebug() << "SENT: ALL STOP ALL STOP ALL STOP";
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
//void LAURobotWidget::onPushButton_clicked()
//{
//    // GET THE STRING FOR THE BUTTON THAT IS EMITTED THE CLICKED SIGNAL
//    QString string = ((QPushButton *)this->sender())->text();

//    // GENERATE A COMMAND STRING FOR THE ROBOT BASED ON WHAT BUTTON THE USER PRESSED
//    if (string == "Forward") {
//        // 0 is super fast reverse, 64 is stop, and 127 is superfast forward
//        //unsigned char val = 60;
//        //emit emitMessage(LAUROBOT_DRIVEMOTOR1_7BIT, &val);

//        unsigned char val = 10;
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
//        emit emitMessage(LAUROBOT_DRIVEBACKWARDSMOTOR2, &val);
//    } else if (string == "Reverse") {
//        unsigned char val = 10;
//        emit emitMessage(LAUROBOT_DRIVEBACKWARDSMOTOR1, &val);
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
//    } else if (string == "Left") {
//        unsigned char val = 10;
//        emit emitMessage(LAUROBOT_DRIVEBACKWARDSMOTOR1, &val);
//        emit emitMessage(LAUROBOT_DRIVEBACKWARDSMOTOR2, &val);
//    } else if (string == "Right") {
//        unsigned char val = 10;
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
//    } else if (string == "Stop") {
//        unsigned char val = 0;
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
//        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
//    }
//}

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
    Q_UNUSED(message);
    Q_UNUSED(argument);
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

//UNKNOWN
        if (message == LAUROBOT_DRIVEFORWARDMOTOR1) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR1);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDMOTOR1";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
// UNKNOWN
        else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR1) {
            byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR1);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEBACKWARDSMOTOR1";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETMAINVOLTAGEMINIMUM) {
        } else if (message == LAUROBOT_SETMAINVOLTAGEMAXIMUM) {
        }
//UNKNOWN
        else if (message == LAUROBOT_DRIVEFORWARDMOTOR2) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR2);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDMOTOR2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//UNKNOWN
        else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR2) {
            byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR2);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEBACKWARDSMOTOR2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_DRIVEMOTOR1_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEMOTOR1_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEMOTOR1_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_DRIVEMOTOR2_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEMOTOR2_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEMOTOR2_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_DRIVEFORWARDMIXEDMODE) {
        } else if (message == LAUROBOT_DRIVEBACKWARDSMIXEDMODE) {
        }
//UNKNOWN
        else if (message == LAUROBOT_TURNRIGHTMIXEDMODE) {
            byteArray.append((char)LAUROBOT_TURNRIGHTMIXEDMODE);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_TURNRIGHTMIXEDMODE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//UNKNOWN
        else if (message == LAUROBOT_TURNLEFTMIXEDMODE) {
        }
//UNKNOWN
        else if (message == LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT) {
            byteArray.append((char)LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//UNKNOWN
        else if (message == LAUROBOT_TURNLEFTORRIGHT_7BIT) {
            byteArray.append((char)LAUROBOT_TURNLEFTORRIGHT_7BIT);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_TURNLEFTORRIGHT_7BIT";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READFIRMWAREVERSION) {
            byteArray.append((char)LAUROBOT_READFIRMWAREVERSION);
            qDebug() << "SENT: LAUROBOT_READFIRMWAREVERSION";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READMAINBATTERYVOLTAGE) {
            byteArray.append((char)LAUROBOT_READMAINBATTERYVOLTAGE);
            qDebug() << "SENT: LAUROBOT_READMAINBATTERYVOLTAGE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//UNKNOWN
        else if (message == LAUROBOT_SETMINLOGICVOLTAGELEVEL) {
            byteArray.append((char)LAUROBOT_SETMINLOGICVOLTAGELEVEL);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_SETMINLOGICVOLTAGELEVEL";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//UNKNOWN
        else if (message == LAUROBOT_SETMAXLOGICVOLTAGELEVEL) {
            byteArray.append((char)LAUROBOT_SETMAXLOGICVOLTAGELEVEL);
            byteArray.append((char *)argument, sizeof(char));
            qDebug() << "SENT: LAUROBOT_SETMAXLOGICVOLTAGELEVEL";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READMOTORPWMS) {
            byteArray.append((char)LAUROBOT_READMOTORPWMS);
            qDebug() << "SENT: LAUROBOT_READMOTORPWMS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READMOTORCURRENTS) {
            byteArray.append((char)LAUROBOT_READMOTORCURRENTS);
            qDebug() << "SENT: LAUROBOT_READMOTORCURRENTS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETMAINBATTERYVOLTAGES) {
        } else if (message == LAUROBOT_SETLOGICBATTERYVOLTAGES) {
        }
//DONE
        else if (message == LAUROBOT_READMAINBATTERYVOLTAGESETTINGS) {
            byteArray.append((char)LAUROBOT_READMAINBATTERYVOLTAGESETTINGS);
            qDebug() << "SENT: LAUROBOT_READMAINBATTERYVOLTAGESETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS) {
            byteArray.append((char)LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS);
            qDebug() << "SENT: LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1) {
        } else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2) {
        } else if (message == LAUROBOT_SETS3S4ANDS5MODES) {
        }
//DONE
        else if (message == LAUROBOT_READS3S4ANDS5MODES) {
            byteArray.append((char)LAUROBOT_READS3S4ANDS5MODES);
            qDebug() << "SENT: LAUROBOT_READS3S4ANDS5MODES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS) {
        }
//DONE
        else if (message == LAUROBOT_READDEADBANDFORRCANALOGCONTROLS) {
            byteArray.append((char)LAUROBOT_READDEADBANDFORRCANALOGCONTROLS);
            qDebug() << "SENT: LAUROBOT_READDEADBANDFORRCANALOGCONTROLS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_RESTOREDEFAULTS) {
        }
// ERROR
        else if (message == LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS) {
            byteArray.append((char)LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS);
            qDebug() << "SENT: LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READTEMPERATURE) {
            byteArray.append((char)LAUROBOT_READTEMPERATURE);
            qDebug() << "SENT: LAUROBOT_READTEMPERATURE";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READTEMPERATURE2) {
            byteArray.append((char)LAUROBOT_READTEMPERATURE);
            qDebug() << "SENT: LAUROBOT_READTEMPERATURE2";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
//DONE
        else if (message == LAUROBOT_READSTATUS) {
            byteArray.append((char)LAUROBOT_READSTATUS);
            qDebug() << "SENT: LAUROBOT_READSTATUS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        }
// ASK
        else if (message == LAUROBOT_READENCODERMODES) {
        } else if (message == LAUROBOT_SETMOTOR1ENCODERMODE) {
        } else if (message == LAUROBOT_SETMOTOR2ENCODERMODE) {
        } else if (message == LAUROBOT_WRITESETTINGSTOEEPROM) {
        }
// ASK
        else if (message == LAUROBOT_READSETTINGSFROMEEPROM) {
        } else if (message == LAUROBOT_SETSTANDARDCONGSETTINGS) {
        }
// ASK
        else if (message == LAUROBOT_READSTANDARDCONGSETTINGS) {
            byteArray.append((char)LAUROBOT_READSTANDARDCONGSETTINGS);
            qDebug() << "SENT: LAUROBOT_READSTANDARDCONGSETTINGS";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETCTRLMODES) {
        }
//DONE
        else if (message == LAUROBOT_READCTRLMODES) {
            byteArray.append((char)LAUROBOT_READCTRLMODES);
            qDebug() << "SENT: LAUROBOT_READCTRLMODES";
            write(appendCRC(byteArray, CRCSend));
            messageIDList.append(message);
        } else if (message == LAUROBOT_SETCTRL1) {
        } else if (message == LAUROBOT_SETCTRL2) {
        }
//TODO
        else if (message == LAUROBOT_READCTRLS) {
        } else if (message == LAUROBOT_SETM1MAXIMUMCURRENT) {
        } else if (message == LAUROBOT_SETM2MAXIMUMCURRENT) {
        }
//TODO
        else if (message == LAUROBOT_READM1MAXIMUMCURRENT) {
        }
//TODO
        else if (message == LAUROBOT_READM2MAXIMUMCURRENT) {
        } else if (message == LAUROBOT_SETPWMMODE) {
        }
//TODO
        else if (message == LAUROBOT_READPWMMODE) {
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
                    qDebug() << "Motor 1 PWMS: "  << m1curr << " A";
                    qDebug() << "Motor 2 PWMS: "  << m2curr << " A";
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
                    uint8_t s3 = ((uint8_t)messageArray.at(0) >> 5);
                    uint8_t s4 = ((uint8_t)messageArray.at(1) >> 5);
                    uint8_t s5 = ((uint8_t)messageArray.at(2) >> 5);
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
                    uint8_t rev = ((uint8_t)messageArray.at(0) >> 5) / 10.0;
                    uint8_t sfor = ((uint8_t)messageArray.at(1) >> 5) / 10.0;
                    qDebug() << "LAUROBOT_READDEADBANDFORRCANALOGCONTROLS: ";
                    qDebug() << "Reverse: "  << rev << "%";
                    qDebug() << "Forward: "  << sfor << "%";
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
            qDebug() << "Length: " << messageArray.length();
            if (messageArray.length() >= 10) {
                if (checkCRC(messageArray.left(10), CRCReceive)){
                    float rev = ((int)messageArray.at(0));
                    float sfor = ((int)messageArray.at(4));
                    qDebug() << "LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS: ";
                    qDebug() << "M1 Duty Cycle Accleration: "  << rev;
                    qDebug() << "M2 Duty Cycle Accleration: "    << sfor;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 10);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READSTATUS: {
            if (messageArray.length() >= 3) {
                if (checkCRC(messageArray.left(3), CRCReceive)){
                    float stat = ((uint8_t)messageArray.at(0) << 4 | (uint8_t)messageArray.at(1));
                    qDebug() << "LAUROBOT_READSTATUS: ";
                    qDebug() << "Status: "  << stat;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READSTATUS message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 3);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        //DOUBLE CHECK
        case LAUROBOT_READSTANDARDCONGSETTINGS: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    uint8_t con = ((uint8_t)messageArray.at(0));
                    qDebug() << "LAUROBOT_READSTANDARDCONGSETTINGS: " << con;
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
        case LAUROBOT_READCTRLMODES: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray.left(4), CRCReceive)){
                    uint8_t con1 = ((uint8_t)messageArray.at(0));
                    uint8_t con2 = ((uint8_t)messageArray.at(1));
                    qDebug() << "LAUROBOT_READCTRLMODES: ";
                    qDebug() << "Control 1: " << con1;
                    qDebug() << "Control 2: " << con2;
                    emit emitMessage(message);
                } else {
                    setError(QString("ERROR receiving LAUROBOT_READCTRLMODES message!"));
                    emit emitError(error());
                }
                messageArray = messageArray.remove(0, 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_DRIVEFORWARDMOTOR1:
        case LAUROBOT_DRIVEFORWARDMOTOR2:
        case LAUROBOT_DRIVEBACKWARDSMOTOR1:
        case LAUROBOT_DRIVEBACKWARDSMOTOR2:
        case LAUROBOT_DRIVEMOTOR1_7BIT:
        case LAUROBOT_DRIVEMOTOR2_7BIT: {
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
    }
    return (false);
}
