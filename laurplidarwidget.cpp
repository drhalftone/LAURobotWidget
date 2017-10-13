#include "laurplidarwidget.h"

const char LAURPLIDAR_RESET_HEADER[7] = { (char)0x52, (char)0x50, (char)0x20, (char)0x4C, (char)0x49, (char)0x44, (char)0x41 };
const char LAURPLIDAR_SCAN_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x05, (char)0x00, (char)0x00, (char)0x40, (char)0x81 };
const char LAURPLIDAR_EXPRESS_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x54, (char)0x00, (char)0x00, (char)0x40, (char)0x82 };
const char LAURPLIDAR_INFO_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x14, (char)0x00, (char)0x00, (char)0x00, (char)0x04 };
const char LAURPLIDAR_HEALTH_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x03, (char)0x00, (char)0x00, (char)0x00, (char)0x06 };
const char LAURPLIDAR_SAMPLERATE[7] = { (char)0xA5, (char)0x5A, (char)0x04, (char)0x00, (char)0x00, (char)0x00, (char)0x15 };

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarWidget::LAURPLidarWidget(QString portString, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAURPLidarWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new LAURPLidarLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new LAURPLidarObject(portString);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        connect(object, SIGNAL(emitScan(QVector<QPoint>)), label, SLOT(onAddPoints(QVector<QPoint>)), Qt::DirectConnection);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarWidget::LAURPLidarWidget(QString ipAddr, int portNum, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAURPLidarWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new LAURPLidarLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->onEnableSavePoints(true);
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new LAURPLidarObject(ipAddr, portNum);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        connect(object, SIGNAL(emitScan(QVector<QPoint>)), label, SLOT(onAddPoints(QVector<QPoint>)), Qt::DirectConnection);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarWidget::~LAURPLidarWidget()
{
    if (object) {
        delete object;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarWidget::showEvent(QShowEvent *)
{
    if (object) {
        object->onInitiateScanning();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onInitiateScanning()
{
    if (isValid()) {
        onReset();
        onGetInfo();
        onGetHealth();
        onGetSampleRate();
        onExpressScan();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarLabel::LAURPLidarLabel(QWidget *parent) : QLabel(parent), savePointsFlag(true), counter(0)
{
    // RESTART THE TIMER
    time.restart();

    // CREATE A CONTEXT MENU FOR TOGGLING TEXTURE
    contextMenu = new QMenu();

    QAction *enableTextureAction = contextMenu->addAction(QString("Save"));
    connect(enableTextureAction, SIGNAL(triggered()), this, SLOT(onSavePoints()));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (contextMenu) {
            contextMenu->popup(event->globalPos());
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::onEnableSavePoints(bool state)
{
    if (state) {
        points.clear();
        savePointsFlag = true;
    } else {
        savePointsFlag = false;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::onAddPoint(QPoint pt)
{
    // REPORT FRAME RATE TO THE CONSOLE
    if (++counter >= 200) {
        //qDebug() << QString("%1 sps").arg(1000.0 * (float)counter / (float)time.elapsed());
        time.restart();
        counter = 0;
    }

    // ONLY ADD POINT IS THE SAVE FLAG IS TRUE
    if (savePointsFlag) {
        // SET THE LIMITS TO INCLUDE THE INCOMING POINT
        topLeft.setX(qMin(topLeft.x(), pt.x()));
        topLeft.setY(qMin(topLeft.y(), pt.y()));

        bottomRight.setX(qMax(bottomRight.x(), pt.x()));
        bottomRight.setY(qMax(bottomRight.y(), pt.y()));

        // ADD THE NEW POINT TO OUR POINT LIST
        points.append(pt);

        // UPDATE THE LABEL ON SCREEN FOR THE USER
        if (points.count() % 100 == 0) {
            //qDebug() << "Number of points:" << points.count();
            update();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::onAddPoints(QList<QPoint> pts)
{
    for (int n = 0; n < pts.count(); n++) {
        onAddPoint(pts.at(n));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::onAddPoints(QVector<QPoint> pts)
{
    for (int n = 0; n < pts.count(); n++) {
        onAddPoint(pts[n]);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::onSavePoints()
{
    // SAVE THE SCANNING DATA TO DISK
    QSettings settings;
    QString directory = settings.value("LAURPLidarWidget::lastDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString filename = QFileDialog::getSaveFileName(this, QString("Save tracking data to disk (*.csv)"), directory, QString("*.csv"));
    if (filename.isEmpty() == false) {
        if (filename.toLower().endsWith(QString(".csv")) == false) {
            filename.append(QString(".csv"));
        }
        settings.setValue("LAURPLidarWidget::lastDirectory", QFileInfo(filename).absolutePath());

        // OPEN A FILE TO SAVE THE RESULTS
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            //file.write(QString("sensorX, sensorY\n").toLatin1());
            for (int n = 0; n < points.count(); n++) {
                QPoint pt = points.at(n);
                file.write(QString("%1, %2\n").arg(pt.x()).arg(pt.y()).toLatin1());
            }
            file.close();
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarLabel::paintEvent(QPaintEvent *)
{
    // CREATE A PAINTER OBJECT TO DRAW ON SCREEN
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // DRAW THE BOUNDING BOX OF THE LABEL IN THE BACKGROUND
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, this->width(), this->height());

    if (points.count() > 100) {
        // CALCULATE SCALE FACTOR TO MAINTAIN 1:1 ASPECT RATIO
        float xScale = (float)this->width() / (float)(bottomRight.x() - topLeft.x());
        float yScale = (float)this->height() / (float)(bottomRight.y() - topLeft.y());

        // SET A TRANSFORM TO KEEP ALL THE POINTS IN THE FIELD OF VIEW OF THE LABEL
        QTransform transformA, transformB, transformC;
        transformA.translate(-(topLeft.x() + bottomRight.x()) / 2, -(topLeft.y() + bottomRight.y()) / 2);
        if (xScale < yScale) {
            transformB.scale(xScale, xScale);
        } else {
            transformB.scale(yScale, yScale);
        }
        transformC.translate(this->width() / 2.0, this->height() / 2.0);
        painter.setTransform(transformA * transformB * transformC);

        // DRAW OUR LIST OF POINTS
        QPen pen(Qt::red, 1.0f, Qt::SolidLine);
        pen.setCosmetic(true);
        painter.setPen(pen);
        for (int n = 1; n < points.count() && n < 10000; n++) {
            painter.drawPoint(points.at(points.count() - n));
        }
    }

    // END DRAWING
    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarObject::~LAURPLidarObject()
{
    if (isValid()) {
        // STOP THE SCANNER
        onStop();

        // WAIT UNTIL ALL MESSAGES HAVE BEEN HANDLED
        while (messageList.count() > 0) {
            qApp->processEvents();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onSendNextMessage()
{
    if (messageList.count() > 0) {
        Packet packet = messageList.first();
        onSendMessage(packet.message, packet.argument);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onSendMessage(int message, void *argument)
{
    Q_UNUSED(argument);

    // CREATE A CHARACTER BUFFER TO HOLD THE MESSAGE
    QByteArray byteArray(1, LAURPLIDAR_FIXED_BYTE);
    switch (message) {
        case LAURPLIDAR_STOP:
            qDebug() << "Send LAURPLIDAR_STOP";
            byteArray.append((char)LAURPLIDAR_STOP);
            write(appendCRC(byteArray));
            messageList.takeFirst();
            QTimer::singleShot(100, this, SLOT(onSendNextMessage()));
            return;
        case LAURPLIDAR_RESET:
            qDebug() << "Send LAURPLIDAR_RESET";
            byteArray.append((char)LAURPLIDAR_RESET);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_SCAN:
            qDebug() << "Send LAURPLIDAR_SCAN";
            byteArray.append((char)LAURPLIDAR_SCAN);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_EXPRESS_SCAN:
            qDebug() << "Send LAURPLIDAR_EXPRESS_SCAN";
            byteArray.append((char)LAURPLIDAR_EXPRESS_SCAN);
            byteArray.append((char)0x05);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            byteArray.append((char)0x00);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_FORCE_SCAN:
            qDebug() << "Send LAURPLIDAR_FORCE_SCAN";
            byteArray.append((char)LAURPLIDAR_FORCE_SCAN);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_GET_INFO:
            qDebug() << "Send LAURPLIDAR_GET_INFO";
            byteArray.append((char)LAURPLIDAR_GET_INFO);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_GET_HEALTH:
            qDebug() << "Send LAURPLIDAR_GET_HEALTH";
            byteArray.append((char)LAURPLIDAR_GET_HEALTH);
            write(appendCRC(byteArray));
            break;
        case LAURPLIDAR_GET_SAMPLERATE:
            qDebug() << "Send LAURPLIDAR_GET_SAMPLERATE";
            byteArray.append((char)LAURPLIDAR_GET_SAMPLERATE);
            write(appendCRC(byteArray));
            break;
    }
    qApp->processEvents();
    waitForBytesWritten(1000);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QByteArray LAURPLidarObject::appendCRC(QByteArray byteArray)
{
    unsigned char byte = 0x00;
    for (int n = 0; n < byteArray.length(); n++) {
        byte ^= byteArray.at(n);
    }
    byteArray.append((char)byte);

    return (byteArray);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onScan()
{
    messageList.append(Packet{LAURPLIDAR_SCAN, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onStop()
{
    messageList.append(Packet{LAURPLIDAR_STOP, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onReset()
{
    messageList.append(Packet{LAURPLIDAR_RESET, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onGetInfo()
{
    messageList.append(Packet{LAURPLIDAR_GET_INFO, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onGetHealth()
{
    messageList.append(Packet{LAURPLIDAR_GET_HEALTH, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onForceScan()
{
    messageList.append(Packet{LAURPLIDAR_FORCE_SCAN, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onExpressScan()
{
    messageList.append(Packet{LAURPLIDAR_EXPRESS_SCAN, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onGetSampleRate()
{
    messageList.append(Packet{LAURPLIDAR_GET_SAMPLERATE, NULL});
    if (messageList.count() == 1) {
        onSendNextMessage();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onReadyRead()
{
    static QByteArray byteArray;

    // KEEP READING FROM THE PORT UNTIL THERE ARE NO MORE BYTES TO READ
    while (bytesAvailable() > 0) {
        byteArray.append(readAll());
        byteArray = processMessage(byteArray);
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QByteArray LAURPLidarObject::processMessage(QByteArray byteArray)
{
    if (byteArray.isEmpty()) {
        return (byteArray);
    }

    if (scanState == StateNotScanning) {
        if (messageList.isEmpty()) {
            return (QByteArray());
        } else if (byteArray.length() < 7) {
            return (byteArray);
        }
        int message = decodeMessageHeader(byteArray);

        // MAKE SURE THIS IS THE MESSAGE WE HAVE BEEN WAITING FOR
        if (message != messageList.first().message) {
            qDebug() << "MESSAGE RECEIVED OUT OF ORDER";
        }

        if (message == LAURPLIDAR_RESET) {
            if (byteArray.length() < 57) {
                return (byteArray);
            } else {
                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 57)));
            }
        } else if (message == LAURPLIDAR_GET_INFO) {
            if (byteArray.length() < 27) {
                return (byteArray);
            } else {
                QByteArray message = byteArray.left(27);

                modelNumber = message.at(7);
                versionMinor = message.at(8);
                versionMajor = message.at(9);
                hardware = message.at(10);
                serialNumber = message.right(16);
                qDebug() << modelNumber << versionMajor << versionMinor << hardware << serialNumber;

                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 27)));
            }
        } else if (message == LAURPLIDAR_GET_HEALTH) {
            if (byteArray.length() < 10) {
                return (byteArray);
            } else {
                QByteArray message = byteArray.left(10);

                int status = message.at(7);
                int errorCode = 256 * (int)message.at(9) + (int)message.at(8);
                qDebug() << status << errorCode;

                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 10)));
            }
        } else if (message == LAURPLIDAR_GET_SAMPLERATE) {
            if (byteArray.length() < 11) {
                return (byteArray);
            } else {
                QByteArray message = byteArray.left(11);

                int sampleRateStandard = 1000000 / (256 * (int)message.at(8) + (int)message.at(7));
                int sampleRateExpress = 1000000 / (256 * (int)message.at(10) + (int)message.at(9));
                qDebug() << sampleRateStandard << sampleRateExpress;

                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 11)));
            }
        } else if (message == LAURPLIDAR_SCAN) {
            if (byteArray.length() < 7) {
                return (byteArray);
            } else {
                scanState = StateScan;
                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 7)));
            }
        } else if (message == LAURPLIDAR_EXPRESS_SCAN) {
            if (byteArray.length() < 7) {
                return (byteArray);
            } else {
                scanState = StateExpressScan;
                messageList.takeFirst();
                onSendNextMessage();
                return (processMessage(byteArray.right(byteArray.length() - 7)));
            }
        } else {
            qDebug() << byteArray;
            return (processMessage(byteArray.right(byteArray.length() - 1)));
        }
    } else if (scanState == StateScan) {
        if (byteArray.length() > 4) {
            bool start = (byteArray.at(0) & 0x02) == 0x02;
            if (start) {
                // BEGINNING OF NEW 360 DEGREE SCAN
            } else {
                // CONTINUATION OF CURRENT 360 DEGREE SCAN
            }
            int quality = byteArray.at(0) & 0xFC;

            int angle = (256 * (int)byteArray.at(2) + (int)byteArray.at(1)) / 2;
            int distance = 256 * (int)byteArray.at(4) + (int)byteArray.at(3);
            qDebug() << quality << angle << distance;

            return (processMessage(byteArray.right(byteArray.length() - 5)));
        }
    } else if (scanState == StateExpressScan) {
        if (byteArray.length() > 7) {
            // LET'S CHECK TO SEE IF WE HAVE A SCAN OR THE RESPONSE TO SOME OTHER MESSAGE
            int message = decodeMessageHeader(byteArray);
            if (message == -1) {
                // IF WE MADE IT THIS FAR, THEN WE HAVE A VALID SCAN
                if (byteArray.length() > 83) {
                    // MAKE SURE WE HAVE THE CORRECT HEADER BYTES FOR THE SCAN
                    if ((byteArray.at(0) & 0xF0) == 0xA0 && (byteArray.at(1) & 0xF0) == 0x50) {
                        // EXTRACT THE HEADER FIELDS OF OUR SCAN
                        bool start = (byteArray.at(3) & 0x80) == 0x80;
                        char checksum = ((byteArray.at(1) & 0x0F) << 4) | (byteArray.at(0) & 0x0F);

                        // GET THE STARTING ANGLE OF THE SCANNER
                        if (start) {
                            angleA = (double)(256 * (int)(byteArray.at(3) & 0x7F) + (int)byteArray.at(2)) / 64.0;
                            angleB = angleA;
                        } else {
                            angleB = angleA;
                            angleA = (double)(256 * (int)(byteArray.at(3) & 0x7F) + (int)byteArray.at(2)) / 64.0;
                        }

                        // ITERATE THROUGH ALL 32 MEASUREMENTS
                        for (int n = 0; n < 16; n++) {
                            // GET THE FIRST OF TWO MEASUREMENTS WITHIN THE CURRENT PAIR
                            double dTheta1 = (16.0 * (int)(byteArray.at(4 + 5 * n + 0) & 0x03) + (int)(byteArray.at(4 + 5 * n + 4) & 0x0F));
                            double distance1 = 64.0 * (int)byteArray.at(4 + 5 * n + 1) + (int)((byteArray.at(4 + 5 * n + 0) >> 1) & 0x3F);
                            scan[2 * n + 0] = getPoint(angleA, angleB, dTheta1, distance1, 2 * n + 0);

                            // GET THE SECOND OF TWO MEASUREMENTS WITHIN THE CURRENT PAIR
                            double dTheta2 = (16.0 * (int)(byteArray.at(4 + 5 * n + 2) & 0x03) + (int)((byteArray.at(4 + 5 * n + 4) & 0xF0) >> 4));
                            double distance2 = 64.0 * (int)byteArray.at(4 + 5 * n + 3) + (int)((byteArray.at(4 + 5 * n + 2) >> 1) & 0x3F);
                            scan[2 * n + 1] = getPoint(angleA, angleB, dTheta2, distance2, 2 * n + 1);
                        }

                        // SEND OUR SCAN VECTOR TO OUR LIDAR LABEL
                        emit emitScan(scan);
                    }
                    // PROCESS WHATEVER REMAINS OF THE INCOMING MESSAGE
                    return (processMessage(byteArray.right(byteArray.length() - 84)));
                }
            } else {
                // IF WE MADE IT HERE, THEN WE MUST NOT HAVE A SCAN BUT SOME OTHER MESSAGE RESPONSE
                scanState = StateNotScanning;
                return (processMessage(byteArray));
            }
        }
    }
    return (byteArray);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QPoint LAURPLidarObject::getPoint(double Aa, double Ab, double dA, double D, int k)
{
    double angleDiff = (Aa > Ab) ? (Aa - Ab) : (360.0 + Aa - Ab);
    double angle = ((double)Aa + (angleDiff * k / 32.0) - dA) * 0.017453292519943;
    return (QPoint((double)D * qCos(angle), (double)D * qSin(angle)));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int LAURPLidarObject::decodeMessageHeader(QByteArray byteArray)
{
    if (byteArray.length() < 7) {
        qDebug() << "NO MESSAGE";
        return (-1);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_RESET_HEADER, 7))) {
        qDebug() << "LAURPLIDAR_RESET_HEADER";
        return (LAURPLIDAR_RESET);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_SCAN_HEADER, 7))) {
        qDebug() << "LAURPLIDAR_SCAN_HEADER";
        return (LAURPLIDAR_SCAN);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_EXPRESS_HEADER, 7))) {
        qDebug() << "LAURPLIDAR_EXPRESS_SCAN";
        return (LAURPLIDAR_EXPRESS_SCAN);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_INFO_HEADER, 7))) {
        qDebug() << "LAURPLIDAR_GET_INFO";
        return (LAURPLIDAR_GET_INFO);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_HEALTH_HEADER, 7))) {
        qDebug() << "LAURPLIDAR_GET_HEALTH";
        return (LAURPLIDAR_GET_HEALTH);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_SAMPLERATE, 7))) {
        qDebug() << "LAURPLIDAR_GET_SAMPLERATE";
        return (LAURPLIDAR_GET_SAMPLERATE);
    } else {
        return (-1);
    }
}
