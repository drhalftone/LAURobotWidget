#include "laurplidarwidget.h"

const char LAURPLIDAR_SCAN_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x05, (char)0x00, (char)0x00, (char)0x40, (char)0x81 };
const char LAURPLIDAR_EXPRESS_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x54, (char)0x00, (char)0x00, (char)0x40, (char)0x82 };
const char LAURPLIDAR_INFO_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x14, (char)0x00, (char)0x00, (char)0x00, (char)0x04 };
const char LAURPLIDAR_HEALTH_HEADER[7] = { (char)0xA5, (char)0x5A, (char)0x03, (char)0x00, (char)0x00, (char)0x00, (char)0x06 };
const char LAURPLIDAR_SAMPLERATE[7] = { (char)0xA5, (char)0x5A, (char)0x04, (char)0x00, (char)0x00, (char)0x00, (char)0x15 };

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarWidget::LAURPLidarWidget(QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAURPLidarWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new LAURPLidarLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new LAURPLidarObject(QString(), 1, NULL);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        //connect(object, SIGNAL(emitPoint(QPoint)), label, SLOT(onAddPoint(QPoint)), Qt::DirectConnection);
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
    if (object->isValid()){
        object->onSendMessage(LAURPLIDAR_SCAN);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAURPLidarLabel::LAURPLidarLabel(QWidget *parent) : QLabel(parent), savePointsFlag(false), counter(0)
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
        pts.clear();
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
        qDebug() << QString("%1 sps").arg(1000.0 * (float)counter / (float)time.elapsed());
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
        pts.append(pt);

        // UPDATE THE LABEL ON SCREEN FOR THE USER
        if (pts.count() % 100 == 0) {
            qDebug() << "Number of points:" << pts.count();
            update();
        }
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
            for (int n = 0; n < pts.count(); n++) {
                QPoint pt = pts.at(n);
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

    if (pts.count() > 100) {
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
        for (int n = 1; n < pts.count() && n < 10000; n++) {
            painter.drawPoint(pts.at(pts.count() - n));
        }
    }

    // END DRAWING
    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAURPLidarObject::onSendMessage(int message, void *argument)
{
    Q_UNUSED(argument);

    // CREATE A CHARACTER BUFFER TO HOLD THE MESSAGE
    QByteArray byteArray(1, LAURPLIDAR_FIXED_BYTE);
    switch (message){
    case LAURPLIDAR_STOP:
        byteArray.append((char)LAURPLIDAR_STOP);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_RESET:
        byteArray.append((char)LAURPLIDAR_RESET);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_SCAN:
        byteArray.append((char)LAURPLIDAR_SCAN);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_EXPRESS_SCAN:
        byteArray.append((char)LAURPLIDAR_EXPRESS_SCAN);
        byteArray.append((char)0x05);
        byteArray.append((char)0x00);
        byteArray.append((char)0x00);
        byteArray.append((char)0x00);
        byteArray.append((char)0x00);
        byteArray.append((char)0x00);
        byteArray.append((char)0x22);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_FORCE_SCAN:
        byteArray.append((char)LAURPLIDAR_FORCE_SCAN);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_GET_INFO:
        byteArray.append((char)LAURPLIDAR_GET_INFO);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_GET_HEALTH:
        byteArray.append((char)LAURPLIDAR_GET_HEALTH);
        write(appendCRC(byteArray));
        break;
    case LAURPLIDAR_GET_SAMPLERATE:
        byteArray.append((char)LAURPLIDAR_GET_SAMPLERATE);
        write(appendCRC(byteArray));
        break;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QByteArray LAURPLidarObject::appendCRC(QByteArray byteArray)
{
    unsigned char byte = 0x00;
    for (int n=0; n<byteArray.length(); n++){
        byte ^= byteArray.at(n);
    }
    byteArray.append((char)byte);

    return(byteArray);
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
    if (byteArray.length() > 2){
        int message = decodeMessageHeader(byteArray);

        if (message == LAURPLIDAR_GET_INFO){
            if (byteArray.length() < 27){
                return(byteArray);
            } else {
                QByteArray message = byteArray.left(27);

                modelNumber = message.at(7);
                versionMinor = message.at(8);
                versionMajor = message.at(9);
                hardware = message.at(10);
                serialNumber = message.right(16);
                qDebug() << modelNumber << versionMajor << versionMinor << hardware << serialNumber;

                return(processMessage(byteArray.right(byteArray.length() - 27)));
            }
        } else if (message == LAURPLIDAR_GET_HEALTH){
            if (byteArray.length() < 10){
                return(byteArray);
            } else {
                QByteArray message = byteArray.left(10);

                int status = message.at(7);
                int errorCode = 256 * (int)message.at(9) + (int)message.at(8);
                qDebug() << status << errorCode;

                return(processMessage(byteArray.right(byteArray.length() - 10)));
            }
        } else if (message == LAURPLIDAR_GET_SAMPLERATE){
            if (byteArray.length() < 11){
                return(byteArray);
            } else {
                QByteArray message = byteArray.left(11);

                int sampleRateStandard = 1000000 / (256 * (int)message.at(8) + (int)message.at(7));
                int sampleRateExpress = 1000000 / (256 * (int)message.at(10) + (int)message.at(9));
                qDebug() << sampleRateStandard << sampleRateExpress;

                return(processMessage(byteArray.right(byteArray.length() - 11)));
            }
        } else if (message == LAURPLIDAR_SCAN){
            if (byteArray.length() < 12){
                return(byteArray);
            } else {
                QByteArray message = byteArray.left(12);

                int quality = message.at(7);
                int angle = 256 * (int)message.at(9) + (int)message.at(8);
                int distance = 256 * (int)message.at(11) + (int)message.at(10);
                qDebug() << quality << angle << distance;

                return(processMessage(byteArray.right(byteArray.length() - 12)));
            }
        }

    }
    return(byteArray);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int LAURPLidarObject::decodeMessageHeader(QByteArray byteArray)
{
    if (byteArray.length() < 7){
        return(-1);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_SCAN_HEADER))){
        return(LAURPLIDAR_SCAN);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_EXPRESS_HEADER))){
        return(LAURPLIDAR_EXPRESS_SCAN);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_INFO_HEADER))){
        return(LAURPLIDAR_GET_INFO);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_HEALTH_HEADER))){
        return(LAURPLIDAR_GET_HEALTH);
    } else if (byteArray.startsWith(QByteArray(LAURPLIDAR_SAMPLERATE))){
        return(LAURPLIDAR_GET_SAMPLERATE);
    }
}
