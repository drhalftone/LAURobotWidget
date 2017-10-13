#include "laupolhumeswidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPolhemusWidget::LAUPolhemusWidget(QString portString, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAUPolhemusWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new LAUPolhemusLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(label);

    // CREATE A POLHEMUS OBJECT FOR CONTROLLING POLHEMUS
    object = new LAUPolhemusObject(portString);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL POLHEMUS OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        connect(object, SIGNAL(emitOdometry(QQuaternion, QVector3D)), label, SLOT(onUpdateOdometry(QQuaternion, QVector3D)), Qt::DirectConnection);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPolhemusWidget::LAUPolhemusWidget(QString ipAddr, int portNum, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("LAUPolhemusWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new LAUPolhemusLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->onEnableSavePoints(true);
    this->layout()->addWidget(label);

    // CREATE A POLHEMUS OBJECT FOR CONTROLLING POLHEMUS
    object = new LAUPolhemusObject(ipAddr, portNum);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL POLHEMUS OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        connect(object, SIGNAL(emitOdometry(QQuaternion, QVector3D)), label, SLOT(onUpdateOdometry(QQuaternion, QVector3D)), Qt::DirectConnection);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPolhemusWidget::~LAUPolhemusWidget()
{
    if (object) {
        delete object;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPolhemusLabel::LAUPolhemusLabel(QWidget *parent) : QLabel(parent), savePointsFlag(false), counter(0)
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
void LAUPolhemusLabel::mousePressEvent(QMouseEvent *event)
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
void LAUPolhemusLabel::onEnableSavePoints(bool state)
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
void LAUPolhemusLabel::onUpdateOdometry(QQuaternion pose, QVector3D position)
{
    qDebug() << pose << position;

    // REPORT FRAME RATE TO THE CONSOLE
    if (++counter >= 200) {
        //qDebug() << QString("%1 sps").arg(1000.0 * (float)counter / (float)time.elapsed());
        time.restart();
        counter = 0;
    }

    // ONLY ADD POINT IS THE SAVE FLAG IS TRUE
    if (savePointsFlag) {
        // SET THE LIMITS TO INCLUDE THE INCOMING POINT
        topLeft.setX(qMin(topLeft.x(), position.x()));
        topLeft.setY(qMin(topLeft.y(), position.y()));
        topLeft.setZ(qMin(topLeft.z(), position.z()));

        bottomRight.setX(qMax(bottomRight.x(), position.x()));
        bottomRight.setY(qMax(bottomRight.y(), position.y()));
        bottomRight.setZ(qMax(bottomRight.z(), position.z()));

        // ADD THE NEW POINT TO OUR POINT LIST
        points.append(position);
        poses.append(pose);

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
void LAUPolhemusLabel::onSavePoints()
{
    // SAVE THE SCANNING DATA TO DISK
    QSettings settings;
    QString directory = settings.value("LAUPolhemusWidget::lastDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString filename = QFileDialog::getSaveFileName(this, QString("Save tracking data to disk (*.csv)"), directory, QString("*.csv"));
    if (filename.isEmpty() == false) {
        if (filename.toLower().endsWith(QString(".csv")) == false) {
            filename.append(QString(".csv"));
        }
        settings.setValue("LAUPolhemusWidget::lastDirectory", QFileInfo(filename).absolutePath());

        // OPEN A FILE TO SAVE THE RESULTS
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            for (int n = 0; n < points.count(); n++) {
                QVector3D pt = points.at(n);
                file.write(QString("%1, %2, %3\n").arg(pt.x()).arg(pt.y()).arg(pt.z()).toLatin1());
            }
            file.close();
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPolhemusWidget::showEvent(QShowEvent *)
{
    object->onSendMessage(LAUPOLHEMUS_CONTINUOUS_PRINT_OUTPUT, NULL);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPolhemusLabel::paintEvent(QPaintEvent *)
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
            painter.drawPoint(points.at(points.count() - n).x(), points.at(points.count() - n).y());
        }
    }

    // END DRAWING
    painter.end();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUPolhemusObject::~LAUPolhemusObject()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPolhemusObject::onSendMessage(int message, void *argument)
{
    switch (message) {
        case LAUPOLHEMUS_CONTINUOUS_PRINT_OUTPUT: {
            QByteArray byteArray(1, LAUPOLHEMUS_CONTINUOUS_PRINT_OUTPUT);
            byteArray.append(0x0D);
            this->write(byteArray);
            break;
        }
        default: {
            break;
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUPolhemusObject::onReadyRead()
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
QByteArray LAUPolhemusObject::processMessage(QByteArray byteArray)
{
    // MAKE SURE WE HAVE ENOUGH BYTES FOR A COMPLETE MESSAGE
    int index = byteArray.indexOf("\r\n", 0);
    if (index > -1) {
        QString string = byteArray.left(index);
        QStringList strings = string.split(" ", QString::SkipEmptyParts);
        if (strings.count() == 7) {
            bool ok = true;
            int sensorID = strings.takeFirst().toInt(&ok, 10);
            if (ok) {
                float xPos = strings.takeFirst().toFloat(&ok);
                if (ok) {
                    float yPos = strings.takeFirst().toFloat(&ok);
                    if (ok) {
                        float zPos = strings.takeFirst().toFloat(&ok);
                        if (ok) {
                            double aAng = strings.takeFirst().toDouble(&ok) * 3.14159265359 / 180.0;
                            if (ok) {
                                double eAng = strings.takeFirst().toDouble(&ok) * 3.14159265359 / 180.0;
                                if (ok) {
                                    double rAng = strings.takeFirst().toDouble(&ok) * 3.14159265359 / 180.0;
                                    if (ok) {
                                        // CREATE THE POSE AND POSITION INSTANCES FROM THE SUPPLIED FLOATING POINT VALUES
                                        QVector3D position(xPos, yPos, zPos);
                                        QQuaternion pose = quaternion(eAng, rAng, aAng);

                                        // EMIT THE POSE AND POSITION TO THE USER
                                        emit emitOdometry(pose, position);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // REMOVE THE POSE AND POSITION FROM THE INCOMING MESSAGE FOR THE NEXT ITERATION
        return (processMessage(byteArray.right(byteArray.length() - (index + 2))));
    }
    return (byteArray);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QQuaternion LAUPolhemusObject::quaternion(double pitch, double roll, double azimuth)
{
    QQuaternion q;

    double cy = qCos(azimuth * 0.5);
    double sy = qSin(azimuth * 0.5);
    double cr = qCos(roll * 0.5);
    double sr = qSin(roll * 0.5);
    double cp = qCos(pitch * 0.5);
    double sp = qSin(pitch * 0.5);

    q.setScalar(cy * cr * cp + sy * sr * sp);
    q.setX(cy * sr * cp - sy * cr * sp);
    q.setY(cy * cr * sp + sy * sr * cp);
    q.setZ(sy * cr * cp - cy * sr * sp);

    return (q);
}
