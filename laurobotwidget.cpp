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
LAURobotWidget::LAURobotWidget(QWidget *parent) : LAUPaletteWidget(QString("RoboClaw"), QList<LAUPalette::Packet>(), parent), robot(NULL)
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
    robot = new LAURobotObject(QString(), 1, NULL);
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
void LAURobotWidget::onValueChanged(QPoint pos, int val)
{
    // DETERMINE IF INCOMING SIGNAL IS FROM LEFT OR RIGHT SLIDER
    if (pos == QPoint(1, 0)) {
        unsigned char uval = (unsigned char)((255 - val) / 2);
        emit emitMessage(LAUROBOT_DRIVEMOTOR1_7BIT, &uval);
    } else if (pos == QPoint(-1, 0)) {
        unsigned char uval = (unsigned char)(val / 2);
        emit emitMessage(LAUROBOT_DRIVEMOTOR2_7BIT, &uval);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onButtonPressed(QPoint pos)
{
    // DETERMINE IF SIGNAL IS FROM PUSH BUTTON
    if (pos == QPoint(0, 1)) {
        unsigned char val = 0;
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotWidget::onButtonReleased(QPoint pos)
{
    // DETERMINE IF SIGNAL IS FROM PUSH BUTTON
    if (pos == QPoint(0, 1)) {
        unsigned char val = 0;
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        emit emitMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
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
    ;
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
LAURobotObject::LAURobotObject(QString portString, QObject *parent) : QObject(parent), ipAddress(portString), portNumber(-1), port(NULL), firmwareString(QString("DEMO"))
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
        portString = QInputDialog::getItem(NULL, QString("RoboClaw"), QString("Select RoboClaw Port"), ports, 0, false, &okay);
        if (okay == false) {
            emit emitError(QString("Connection to RoboClaw canceled by user."));
        }
    }

    if (portString.isEmpty() == false) {
        // CREATE A SERIAL PORT OBJECT
        port = new QSerialPort();

        // SET THE SERIAL PORT SETTINGS
        ((QSerialPort *)port)->setPortName(portString);
        ((QSerialPort *)port)->setBaudRate(QSerialPort::Baud115200);
        ((QSerialPort *)port)->setDataBits(QSerialPort::Data8);
        ((QSerialPort *)port)->setStopBits(QSerialPort::OneStop);
        ((QSerialPort *)port)->setParity(QSerialPort::NoParity);
        ((QSerialPort *)port)->setFlowControl(QSerialPort::NoFlowControl);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURobotObject::LAURobotObject(QString ipAddr, int portNum, QObject *parent) : QObject(parent), ipAddress(ipAddr), portNumber(portNum), port(NULL), firmwareString(QString("DEMO"))
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
bool LAURobotObject::connectPort()
{
    // MAKE SURE WE HAVE WHAT WE NEED FOR A CONNECTION
    if (port) {
        // CREATE A NEW CONNECTION OR OPEN THE SERIAL PORT FOR COMMUNICATION
        if (dynamic_cast<QTcpSocket *>(port) != NULL) {
            ((QTcpSocket *)port)->connectToHost(ipAddress, portNumber, QIODevice::ReadWrite);
            if (((QTcpSocket *)port)->waitForConnected(3000) == false) {
                errorString = QString("Cannot connect to RoboClaw.\n") + port->errorString();
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
                errorString = QString("Cannot connect to RoboClaw.\n") + port->errorString();
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
LAURobotObject::~LAURobotObject()
{
    // CLOSE THE SERIAL PORT IF IT IS CURRENTLY OPEN
    if (port && port->isOpen()) {
        unsigned char val = 0;
        onSendMessage(LAUROBOT_DRIVEFORWARDMOTOR1, &val);
        onSendMessage(LAUROBOT_DRIVEFORWARDMOTOR2, &val);
        port->waitForBytesWritten(1000);
        qApp->processEvents();
        port->close();
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
    if (!port || port->isOpen() == false) {
        return;
    }

    // CREATE A CHARACTER BUFFER TO HOLD THE MESSAGE
    QByteArray byteArray(1, LAUROBOT_WIDGETADDRESS);

    if (message == LAUROBOT_DRIVEFORWARDMOTOR1) {
        byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR1);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR1) {
        byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR1);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_SETMAINVOLTAGEMINIMUM) {
    } else if (message == LAUROBOT_SETMAINVOLTAGEMAXIMUM) {
    } else if (message == LAUROBOT_DRIVEFORWARDMOTOR2) {
        byteArray.append((char)LAUROBOT_DRIVEFORWARDMOTOR2);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_DRIVEBACKWARDSMOTOR2) {
        byteArray.append((char)LAUROBOT_DRIVEBACKWARDSMOTOR2);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_DRIVEMOTOR1_7BIT) {
        byteArray.append((char)LAUROBOT_DRIVEMOTOR1_7BIT);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_DRIVEMOTOR2_7BIT) {
        byteArray.append((char)LAUROBOT_DRIVEMOTOR2_7BIT);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_DRIVEFORWARDMIXEDMODE) {
    } else if (message == LAUROBOT_DRIVEBACKWARDSMIXEDMODE) {
    } else if (message == LAUROBOT_TURNRIGHTMIXEDMODE) {
    } else if (message == LAUROBOT_TURNLEFTMIXEDMODE) {
    } else if (message == LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT) {
        byteArray.append((char)LAUROBOT_DRIVEFORWARDORBACKWARD_7BIT);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_TURNLEFTORRIGHT_7BIT) {
        byteArray.append((char)LAUROBOT_TURNLEFTORRIGHT_7BIT);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_READFIRMWAREVERSION) {
        byteArray.append((char)LAUROBOT_READFIRMWAREVERSION);
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_READMAINBATTERYVOLTAGE) {
        byteArray.append((char)LAUROBOT_READMAINBATTERYVOLTAGE);
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_SETMINLOGICVOLTAGELEVEL) {
        byteArray.append((char)LAUROBOT_SETMINLOGICVOLTAGELEVEL);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_SETMAXLOGICVOLTAGELEVEL) {
        byteArray.append((char)LAUROBOT_SETMAXLOGICVOLTAGELEVEL);
        byteArray.append((char *)argument, sizeof(char));
        port->write(appendCRC(byteArray, CRCSend));
        messageIDList.append(message);
    } else if (message == LAUROBOT_READMOTORPWMS) {
    } else if (message == LAUROBOT_READMOTORCURRENTS) {
    } else if (message == LAUROBOT_SETMAINBATTERYVOLTAGES) {
    } else if (message == LAUROBOT_SETLOGICBATTERYVOLTAGES) {
    } else if (message == LAUROBOT_READMAINBATTERYVOLTAGESETTINGS) {
    } else if (message == LAUROBOT_READLOGICBATTERYVOLTAGESETTINGS) {
    } else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM1) {
    } else if (message == LAUROBOT_SETDEFAULTDUTYCYCLEACCELERATIONFORM2) {
    } else if (message == LAUROBOT_SETS3S4ANDS5MODES) {
    } else if (message == LAUROBOT_READS3S4ANDS5MODES) {
    } else if (message == LAUROBOT_SETDEADBANDFORRCANALOGCONTROLS) {
    } else if (message == LAUROBOT_READDEADBANDFORRCANALOGCONTROLS) {
    } else if (message == LAUROBOT_RESTOREDEFAULTS) {
    } else if (message == LAUROBOT_READDEFAULTDUTYCYCLEACCELERATIONS) {
    } else if (message == LAUROBOT_READTEMPERATURE) {
    } else if (message == LAUROBOT_READTEMPERATURE2) {
    } else if (message == LAUROBOT_READSTATUS) {
    } else if (message == LAUROBOT_READENCODERMODES) {
    } else if (message == LAUROBOT_SETMOTOR1ENCODERMODE) {
    } else if (message == LAUROBOT_SETMOTOR2ENCODERMODE) {
    } else if (message == LAUROBOT_WRITESETTINGSTOEEPROM) {
    } else if (message == LAUROBOT_READSETTINGSFROMEEPROM) {
    } else if (message == LAUROBOT_SETSTANDARDCONGSETTINGS) {
    } else if (message == LAUROBOT_READSTANDARDCONGSETTINGS) {
    } else if (message == LAUROBOT_SETCTRLMODES) {
    } else if (message == LAUROBOT_READCTRLMODES) {
    } else if (message == LAUROBOT_SETCTRL1) {
    } else if (message == LAUROBOT_SETCTRL2) {
    } else if (message == LAUROBOT_READCTRLS) {
    } else if (message == LAUROBOT_SETM1MAXIMUMCURRENT) {
    } else if (message == LAUROBOT_SETM2MAXIMUMCURRENT) {
    } else if (message == LAUROBOT_READM1MAXIMUMCURRENT) {
    } else if (message == LAUROBOT_READM2MAXIMUMCURRENT) {
    } else if (message == LAUROBOT_SETPWMMODE) {
    } else if (message == LAUROBOT_READPWMMODE) {
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURobotObject::onReadyRead()
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
                if (checkCRC(messageArray, CRCReceive)) {
                    firmwareString = QString(messageArray.left(index));
                    emit emitMessage(message);
                } else {
                    errorString = QString("ERROR receiving LAUROBOT_READFIRMWAREVERSION message!");
                    emit emitError(errorString);
                }
                messageArray = messageArray.remove(0, index + 4);
                messageIDList.takeFirst();
                return (true);
            }
            break;
        }
        case LAUROBOT_READMAINBATTERYVOLTAGE: {
            if (messageArray.length() >= 4) {
                if (checkCRC(messageArray, CRCReceive)) {
                    emit emitMessage(message);
                } else {
                    errorString = QString("ERROR receiving LAUROBOT_READMAINBATTERYVOLTAGE message!");
                    emit emitError(errorString);
                }
                messageArray = messageArray.remove(0, 3);
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
        case LAUROBOT_DRIVEMOTOR2_7BIT:
        case LAUROBOT_TURNLEFTORRIGHT_7BIT: {
            if (messageArray.length() >= 1) {
                if ((unsigned char)messageArray.at(0) == 0xff) {
                    emit emitMessage(message);
                } else {
                    errorString = QString("ERROR sending LAUROBOT_DRIVEFORWARDMOTOR1 message!");
                    emit emitError(errorString);
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
