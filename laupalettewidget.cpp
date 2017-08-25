#include "laupalettewidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPaletteWidget::LAUPaletteWidget(QString string, QList<LAUPalette::Packet> list, QWidget *parent) : QWidget(parent), palette(NULL), paletteState(-1), packets(list), deviceString(string)
{
    // SET THE WIDGET LAYOUT
    this->setWindowTitle(deviceString);
    this->setLayout(new QVBoxLayout());
    this->setFixedSize(75, 75);
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // ADD A PALETTE LABEL TO DRAW A PICTURE OF THE PALETTE LAYOUT
    LAUPaletteLabel *label = new LAUPaletteLabel;
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    palette = new LAUPalette(QString());

    connect(palette, SIGNAL(emitConnected()), this, SLOT(onConnected()));
    connect(palette, SIGNAL(emitDisconnected()), this, SLOT(onDisconnected()));
    connect(palette, SIGNAL(emitError(QString)), this, SLOT(onError(QString)));
    connect(palette, SIGNAL(emitValueChanged(QPoint, int)), this, SLOT(onValueChanged(QPoint, int)));
    connect(palette, SIGNAL(emitDialRotated(QPoint, int)), this, SLOT(onDialRotated(QPoint, int)));
    connect(palette, SIGNAL(emitButtonPressed(QPoint)), this, SLOT(onButtonPressed(QPoint)));
    connect(palette, SIGNAL(emitButtonReleased(QPoint)), this, SLOT(onButtonReleased(QPoint)));
    connect(palette, SIGNAL(emitError(QString)), this, SLOT(onError(QString)));

    connect(palette, SIGNAL(emitUpdate()), label, SLOT(update()));
    label->setPalette(palette);

    // TELL THE PALETTE OBJECT TO CONNECT TO THE DEVICE
    if (palette) {
        palette->connectPort();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPaletteWidget::~LAUPaletteWidget()
{
    if (palette) {
        delete palette;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::registerLayout(QList<LAUPalette::Packet> list)
{
    // KEEP A LOCAL COPY OF THE PACKET LIST
    packets = list;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onConnected()
{
    // RESIZE THE WIDGET FOR THE CURRENT PALETTE LAYOUT
    QRect rect = palette->boundingBox();
    this->setFixedSize(rect.width() * LAUPALETTEOBJECTSIZE + LAUPALETTEOBJECTSIZE, rect.height() * LAUPALETTEOBJECTSIZE + LAUPALETTEOBJECTSIZE);
    this->registerLayout(packets);

    // IF WE HAVE A PACKET LIST, THEN TEST IT WITH THE INCOMING LAYOUT NOW THAT WE ARE CONNECTED
    if (palette->registerLayout(packets)) {
        if (paletteState != 1) {
            paletteState = 1;
            QMessageBox::information(this, deviceString, QString("Ready for %1!").arg(deviceString));
        }
        // CALL A VIRTUAL METHOD SO THAT THE SUBCLASS WIDGET CAN RESPOND ACCORDINGLY
        paletteConnected();
    } else {
        if (paletteState != 0) {
            paletteState = 0;
            QMessageBox::warning(this, deviceString, QString("Not ready for %1!").arg(deviceString));
        }
        // CALL A VIRTUAL METHOD SO THAT THE SUBCLASS WIDGET CAN RESPOND ACCORDINGLY
        paletteDisconnected();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onDisconnected()
{
    // RESIZE THE WIDGET FOR THE CURRENT PALETTE LAYOUT
    this->setFixedSize(LAUPALETTEOBJECTSIZE, LAUPALETTEOBJECTSIZE);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onError(QString string)
{
    QMessageBox::warning(this, QString("Palette Error"), string);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onValueChanged(QPoint pos, int val)
{
    qDebug() << "emitValueChanged(packets.at(n).pos, val)" << pos << val;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onDialRotated(QPoint pos, int val)
{
    qDebug() << "emitDialRotated(packets.at(n).pos, val)" << pos << val;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onButtonPressed(QPoint pos)
{
    qDebug() << "emitButtonPressed(packets.at(n).pos)" << pos;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteWidget::onButtonReleased(QPoint pos)
{
    qDebug() << "emitButtonReleased(packets.at(n).pos)" << pos;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPalette::LAUPalette(QString portString, QObject *parent) : QObject(parent), ipAddress(portString), portNumber(-1), port(NULL)
{
    if (portString.isEmpty()) {
        QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
        for (int n = 0; n < portList.count(); n++) {
            if (portList.at(n).vendorIdentifier() == 0x16d0 && portList.at(n).productIdentifier() == 0x09F8) {
                portString = portList.at(n).portName();
            }
        }
    }

    if (portString.isEmpty() == false) {
        // CREATE A SERIAL PORT OBJECT
        port = new QSerialPort();

        ((QSerialPort *)port)->setPortName(portString);
        ((QSerialPort *)port)->setBaudRate(QSerialPort::Baud115200);
        ((QSerialPort *)port)->setDataBits(QSerialPort::Data8);
        ((QSerialPort *)port)->setStopBits(QSerialPort::OneStop);
        ((QSerialPort *)port)->setParity(QSerialPort::NoParity);
        ((QSerialPort *)port)->setFlowControl(QSerialPort::NoFlowControl);
        connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPalette::LAUPalette(QString ipAddr, int portNum, QObject *parent) : QObject(parent), ipAddress(ipAddr), portNumber(portNum), port(NULL)
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
LAUPalette::~LAUPalette()
{
    while (palettes.isEmpty() == false) {
        delete palettes.takeFirst();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUPalette::connectPort()
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
                port->write(QJsonDocument(QJsonObject{{"start", 1}}).toJson());
                return (true);
            }
        } else if (dynamic_cast<QSerialPort *>(port) != NULL) {
            if (!port->open(QIODevice::ReadWrite)) {
                errorString = QString("Cannot connect to RoboClaw.\n") + port->errorString();
                emit emitError(errorString);
            } else if (!port->isReadable()) {
                errorString = QString("Port is not readable!\n") + port->errorString();
                emit emitError(errorString);
            } else if (!port->isWritable()) {
                errorString = QString("Port is not writable!\n") + port->errorString();
                emit emitError(errorString);
            } else {
                port->write(QJsonDocument(QJsonObject{{"start", 1}}).toJson());
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
void LAUPalette::onConnected()
{
    emit emitConnected();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onDisconnected()
{
    emit emitDisconnected();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onTcpError(QAbstractSocket::SocketError error)
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
void LAUPalette::onReadyRead()
{
    static QByteArray message;

    while (port->bytesAvailable() > 0) {
        QByteArray byteArray = port->readAll();
        if (!byteArray.isEmpty()) {
            // ADD THE INCOMING MESSAGE TO OUR RUNNING BUFFER
            message.append(byteArray);

            while (message.contains('\n')) {
                int index = message.indexOf('\n');
                byteArray = message.left(index + 1);
                message = message.right(message.length() - index - 1);

                QJsonParseError error;
                QJsonDocument document = QJsonDocument::fromJson(byteArray, &error);
                if (error.error == QJsonParseError::NoError) {
                    if (document.isObject()) {
                        QJsonObject objectA = document.object();
                        if (objectA.contains(QString("l"))) {
                            while (palettes.isEmpty() == false) {
                                delete palettes.takeFirst();
                            }
                            processLayout(objectA.value(QString("l")).toObject());
                            emit emitConnected();
                            emit emitUpdate();
                        } else {
                            QVariantMap mapA = objectA.toVariantMap();
                            for (int n = 0; n < mapA.value(QString("in")).toList().count(); n++) {
                                QVariantMap mapB = mapA.value(QString("in")).toList()[n].toMap();
                                QList<QVariant> mapC = mapB.value(QString("v")).toList();

                                QList<int> samples;
                                for (int n = 0; n < mapC.length(); n++) {
                                    samples << mapC.at(n).toInt();
                                }
                                unsigned int id = mapB.value(QString("i")).toInt();
                                for (int n = 0; n < palettes.count(); n++) {
                                    if (palettes.at(n)->identity() == id) {
                                        palettes.at(n)->onValueChanged(id, samples);
                                        break;
                                    }
                                }
                            }
                            emit emitUpdate();
                        }
                    } else {
                        ;
                    }
                }
            }
        }
    }

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onValueChanged(int val)
{
    for (int n = 0; n < packets.count(); n++) {
        if (QObject::sender() == whatsAt(packets.at(n).pos)) {
            emit emitValueChanged(packets.at(n).pos, val);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onDialRotated(int val)
{
    for (int n = 0; n < packets.count(); n++) {
        if (QObject::sender() == whatsAt(packets.at(n).pos)) {
            emit emitDialRotated(packets.at(n).pos, val);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onButtonPressed()
{
    for (int n = 0; n < packets.count(); n++) {
        if (QObject::sender() == whatsAt(packets.at(n).pos)) {
            emit emitButtonPressed(packets.at(n).pos);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::onButtonReleased()
{
    for (int n = 0; n < packets.count(); n++) {
        if (QObject::sender() == whatsAt(packets.at(n).pos)) {
            emit emitButtonReleased(packets.at(n).pos);
        }
    }
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPalette::processLayout(QJsonObject object, QTransform transform)
{
    // CHECK FOR THE NULL DEVICE
    if (object.isEmpty()) {
        return;
    }

    // SEE WHAT KIND OF PALETTE DEVICE WE HAVE
    int id = object.value(QString("i")).toInt();
    QJsonArray children = object.value(QString("c")).toArray();
    LAUPaletteObject *palette = NULL;
    if (object.value(QString("t")) == LAUPaletteObject::PaletteBase) {
        palette = new LAUPaletteBase(id);

        // CIRCULATE THROUGH THE CHILD DEVICES
        processLayout(children.at(0).toObject(), palette->neighborTransform(0, transform)); // RIGHT
        processLayout(children.at(1).toObject(), palette->neighborTransform(1, transform)); // BELOW
        processLayout(children.at(2).toObject(), palette->neighborTransform(2, transform)); // LEFT
    } else if (object.value(QString("t")) == LAUPaletteObject::PaletteButton) {
        palette = new LAUPaletteButton(id);

        // CIRCULATE THROUGH THE CHILD DEVICES
        processLayout(children.at(0).toObject(), palette->neighborTransform(0, transform)); // RIGHT
        processLayout(children.at(1).toObject(), palette->neighborTransform(1, transform)); // BELOW
        processLayout(children.at(2).toObject(), palette->neighborTransform(2, transform)); // LEFT
    } else if (object.value(QString("t")) == LAUPaletteObject::PaletteDial) {
        palette = new LAUPaletteDial(id);

        // CIRCULATE THROUGH THE CHILD DEVICES
        processLayout(children.at(0).toObject(), palette->neighborTransform(0, transform)); // RIGHT
        processLayout(children.at(1).toObject(), palette->neighborTransform(1, transform)); // BELOW
        processLayout(children.at(2).toObject(), palette->neighborTransform(2, transform)); // LEFT
    } else if (object.value(QString("t")) == LAUPaletteObject::PaletteSlider) {
        palette = new LAUPaletteSlider(id);

        // CIRCULATE THROUGH THE CHILD DEVICES
        processLayout(children.at(0).toObject(), palette->neighborTransform(3, transform)); // TOP-RIGHT
        processLayout(children.at(1).toObject(), palette->neighborTransform(4, transform)); // RIGHT
        processLayout(children.at(2).toObject(), palette->neighborTransform(5, transform)); // DOWN-RIGHT
        processLayout(children.at(3).toObject(), palette->neighborTransform(1, transform)); // BELOW
        processLayout(children.at(4).toObject(), palette->neighborTransform(2, transform)); // LEFT
    }
    palette->setPosition(transform);
    palettes << palette;
    qDebug() << palette->location() << palette->orientation();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QRect LAUPalette::boundingBox()
{
    int xMin = 1000, xMax = -1000;
    int yMin = 1000, yMax = -1000;
    for (int n = 0; n < palettes.count(); n++) {
        xMin = qMin(palettes.at(n)->location(0).x(), xMin);
        xMax = qMax(palettes.at(n)->location(0).x(), xMax);
        yMin = qMin(palettes.at(n)->location(0).y(), yMin);
        yMax = qMax(palettes.at(n)->location(0).y(), yMax);

        if (palettes.at(n)->type() == LAUPaletteObject::PaletteSlider) {
            xMin = qMin(palettes.at(n)->location(1).x(), xMin);
            xMax = qMax(palettes.at(n)->location(1).x(), xMax);
            yMin = qMin(palettes.at(n)->location(1).y(), yMin);
            yMax = qMax(palettes.at(n)->location(1).y(), yMax);
        }
    }
    return (QRect(xMin, yMin, xMax - xMin, yMax - yMin));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUPalette::registerLayout(QList<LAUPalette::Packet> list)
{
    // KEEP A LOCAL COPY OF THE PACKET LIST
    packets = list;

    // IF WE HAVE A PACKET LIST, THEN TEST IT WITH THE INCOMING LAYOUT NOW THAT WE ARE CONNECTED
    if (packets.count() > 0) {
        // CHECK TO SEE IF WE HAVE THE LAYOUT WE NEED
        bool flag = true;
        for (int n = 0; n < packets.count(); n++) {
            LAUPaletteObject *object = whatsAt(packets.at(n).pos);
            if (!object || object->type() != packets.at(n).pal) {
                return (false);
            }
        }
        // ASSUMING WE DO HAVE WHAT WE NEED, THEN CONNECT ALL THE DEVICES
        // OTHERWISE NO DEVICES ARE CONNECTED AND NO SIGNALS SENT TO THE PALETTE WIDGET
        if (flag) {
            for (int n = 0; n < packets.count(); n++) {
                LAUPaletteObject *object = whatsAt(packets.at(n).pos);
                connect(object, SIGNAL(emitValueChanged(int)), this, SLOT(onValueChanged(int)));
                connect(object, SIGNAL(emitDialRotated(int)), this, SLOT(onDialRotated(int)));
                connect(object, SIGNAL(emitButtonReleased()), this, SLOT(onButtonReleased()));
                connect(object, SIGNAL(emitButtonPressed()), this, SLOT(onButtonPressed()));
            }
            return (true);
        }
    }
    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPaletteObject *LAUPalette::whatsAt(QPoint point)
{
    for (int n = 0; n < palettes.count(); n++) {
        if (palettes.at(n)->location(0) == point) {
            return (palettes.at(n));
        } else if (palettes.at(n)->location(1) == point) {
            return (palettes.at(n));
        }
    }
    return (NULL);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QTransform LAUPaletteObject::neighborTransform(int i, QTransform transform)
{
    switch (i) {
        case 0: // RIGHT
            return (QTransform().translate((float)LAUPALETTEOBJECTSIZE, 0.0f).rotate(-90.0f) * transform);
        case 1: // DOWN
            return (QTransform().translate(0.0f, (float)LAUPALETTEOBJECTSIZE) * transform);
        case 2: // LEFT
            return (QTransform().translate(-(float)LAUPALETTEOBJECTSIZE, 0.0f).rotate(90.0f) * transform);
        case 3: // RIGHT & UP
            return (QTransform().translate((float)LAUPALETTEOBJECTSIZE, -(float)LAUPALETTEOBJECTSIZE).rotate(180.0f) * transform);
        case 4: // RIGHT & RIGHT
            return (QTransform().translate((float)(2 * LAUPALETTEOBJECTSIZE), 0.0f).rotate(-90.0f) * transform);
        case 5: // RIGHT & DOWN
            return (QTransform().translate((float)LAUPALETTEOBJECTSIZE, (float)LAUPALETTEOBJECTSIZE) * transform);
        default:
            return (transform);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPaletteObject::Orientation LAUPaletteObject::orientation() const
{
    if (transform.m11() > 0.5 && transform.m22() > 0.5) {
        return (OrientationA);
    } else if (transform.m12() < -0.5 && transform.m21() > 0.5) {
        return (OrientationB);
    } else if (transform.m12() > 0.5 && transform.m21() < -0.5) {
        return (OrientationC);
    } else if (transform.m11() < -0.5 && transform.m22() < -0.5) {
        return (OrientationD);
    }
    return (OrientationE);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QPoint LAUPaletteObject::location(int a) const
{
    Q_UNUSED(a);
    return (QPoint(qRound(transform.dx() / (float)LAUPALETTEOBJECTSIZE), qRound(transform.dy() / (float)LAUPALETTEOBJECTSIZE)));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteBase::draw(QPainter *painter) const
{
    QTransform transform = painter->transform();
    painter->setTransform(transform * position());

    painter->setPen(QPen(QColor(196, 196, 196), 2));
    painter->setBrush(QBrush(QColor(64, 64, 64), Qt::SolidPattern));
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2, -LAUPALETTEOBJECTSIZE / 2, LAUPALETTEOBJECTSIZE, LAUPALETTEOBJECTSIZE, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    painter->drawPixmap(-LAUPALETTEOBJECTSIZE / 2 + 10, -LAUPALETTEOBJECTSIZE / 2 + 10, LAUPALETTEOBJECTSIZE - 20, LAUPALETTEOBJECTSIZE - 20, QPixmap::fromImage(QImage(":/Images/Images/Palette.bmp")));

    painter->setPen(QPen(QColor(0, 164, 164), 4 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2 + 5, -LAUPALETTEOBJECTSIZE / 2 + 5, LAUPALETTEOBJECTSIZE - 10, LAUPALETTEOBJECTSIZE - 10, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    painter->setTransform(transform);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteButton::draw(QPainter *painter) const
{
    QTransform transform = painter->transform();
    painter->setTransform(position() * transform);

    painter->setPen(QPen(QColor(196, 196, 196), 2));
    painter->setBrush(QBrush(QColor(64, 64, 64), Qt::SolidPattern));
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2, -LAUPALETTEOBJECTSIZE / 2, LAUPALETTEOBJECTSIZE, LAUPALETTEOBJECTSIZE, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    if (value == 0) {
        painter->setPen(QPen(QColor(196, 0, 0), 10 * LAUPALETTEOBJECTLAMBDA));
        painter->setBrush(QBrush(QColor(255, 0, 0), Qt::SolidPattern));
        painter->drawEllipse(-LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, -LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA);
    } else {
        painter->setPen(QPen(QColor(196, 0, 0), 10 * LAUPALETTEOBJECTLAMBDA));
        painter->setBrush(QBrush(QColor(96, 0, 0), Qt::SolidPattern));
        painter->drawEllipse(-LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, -LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA);
    }

    painter->setPen(QPen(QColor(0, 164, 164), 4 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2 + 5, -LAUPALETTEOBJECTSIZE / 2 + 5, LAUPALETTEOBJECTSIZE - 10, LAUPALETTEOBJECTSIZE - 10, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    painter->setTransform(transform);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteDial::draw(QPainter *painter) const
{
    QTransform transform = painter->transform();
    painter->setTransform(QTransform().translate((position() * transform).dx(), (position() * transform).dy()));
    painter->setPen(QPen(QColor(196, 196, 196), 2));
    painter->setBrush(QBrush(QColor(64, 64, 64), Qt::SolidPattern));
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2, -LAUPALETTEOBJECTSIZE / 2, LAUPALETTEOBJECTSIZE, LAUPALETTEOBJECTSIZE, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    if (upDownFlag) {
        painter->setPen(QPen(QColor(0, 0, 0), 1));
        painter->setBrush(QBrush(QColor(0, 0, 0), Qt::SolidPattern));
        painter->drawEllipse(-LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, -LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA);
    } else {
        painter->setPen(QPen(QColor(255, 255, 255), 1));
        painter->setBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
        painter->drawEllipse(-LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, -LAUPALETTEOBJECTSIZE / 2 + 20 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA, 60 * LAUPALETTEOBJECTLAMBDA);
    }

    painter->setPen(QPen(QColor(0, 164, 164), 4 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2 + 5, -LAUPALETTEOBJECTSIZE / 2 + 5, LAUPALETTEOBJECTSIZE - 10, LAUPALETTEOBJECTSIZE - 10, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    QTransform transformB = painter->transform();
    transformB.rotate((float)value / 255.0f * 340.0f + 10.0f);
    painter->setTransform(transformB);
    painter->setPen(QPen(QColor(0, 0, 0), 1));
    painter->setBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
    painter->drawEllipse(-LAUPALETTEOBJECTSIZE / 20, 22 * LAUPALETTEOBJECTLAMBDA, LAUPALETTEOBJECTSIZE / 10, LAUPALETTEOBJECTSIZE / 10);

    painter->setTransform(transform);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteSlider::draw(QPainter *painter) const
{
    QTransform transform = painter->transform();
    painter->setTransform(position() * transform);

    painter->setPen(QPen(QColor(196, 196, 196), 2));
    painter->setBrush(QBrush(QColor(64, 64, 64), Qt::SolidPattern));
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2, -LAUPALETTEOBJECTSIZE / 2, 2 * LAUPALETTEOBJECTSIZE, LAUPALETTEOBJECTSIZE, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    painter->setPen(QPen(Qt::black, 5 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(-20 * LAUPALETTEOBJECTLAMBDA, 0, 20 * LAUPALETTEOBJECTLAMBDA + LAUPALETTEOBJECTSIZE, 0);

    int posX = (int)qRound((float)value / 255.0f * (40 * LAUPALETTEOBJECTLAMBDA + LAUPALETTEOBJECTSIZE));
    painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    painter->drawRect(posX - 30 * LAUPALETTEOBJECTLAMBDA, -25 * LAUPALETTEOBJECTLAMBDA, 20 * LAUPALETTEOBJECTLAMBDA, 50 * LAUPALETTEOBJECTLAMBDA);

    painter->setPen(QPen(Qt::white, 3 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(posX - 20 * LAUPALETTEOBJECTLAMBDA, -25 * LAUPALETTEOBJECTLAMBDA, posX - 20 * LAUPALETTEOBJECTLAMBDA, 25 * LAUPALETTEOBJECTLAMBDA);

    painter->setPen(QPen(QColor(0, 164, 164), 4 * LAUPALETTEOBJECTLAMBDA));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(-LAUPALETTEOBJECTSIZE / 2 + 5, -LAUPALETTEOBJECTSIZE / 2 + 5, 2 * LAUPALETTEOBJECTSIZE - 10, LAUPALETTEOBJECTSIZE - 10, 5 * LAUPALETTEOBJECTLAMBDA, 5 * LAUPALETTEOBJECTLAMBDA);

    painter->setTransform(transform);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QPoint LAUPaletteSlider::location(int a) const
{
    if (a == 0) {
        return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE), qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE)));
    } else {
        switch (orientation()) {
            case OrientationA:
                return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE + 1), qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE)));
            case OrientationB:
                return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE), qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE) - 1));
            case OrientationC:
                return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE), qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE) + 1));
            case OrientationD:
                return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE) - 1, qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE)));
            default:
                return (QPoint(qRound(position().dx() / (float)LAUPALETTEOBJECTSIZE), qRound(position().dy() / (float)LAUPALETTEOBJECTSIZE)));
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPaletteLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
    painter.drawRect(0, 0, width(), height());

    QRect rect = palette->boundingBox();

    QTransform transform;
    transform.translate(-rect.left() * LAUPALETTEOBJECTSIZE + LAUPALETTEOBJECTSIZE / 2, -rect.top() * LAUPALETTEOBJECTSIZE + LAUPALETTEOBJECTSIZE / 2);
    painter.setTransform(transform);

    palette->draw(&painter);
}
