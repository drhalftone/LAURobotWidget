#include "austincolorcamwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
AustinColorCamWidget::AustinColorCamWidget(QString portString, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("AustinColorCamWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new AustinColorCamLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new AustinColorCamObject(portString);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
    if (object->connectPort()) {
        connect(object, SIGNAL(emitPoint(QPoint)), label, SLOT(onAddPoint(QPoint)), Qt::DirectConnection);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
AustinColorCamWidget::AustinColorCamWidget(QString ipAddr, int portNum, QWidget *parent) : QWidget(parent), object(NULL)
{
    // SET THE WINDOWS LAYOUT
    this->setWindowTitle("AustinColorCamWidget");
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);

    // CREATE LIDAR LABEL TO DISPLAY LIDAR DATA AS IT ARRIVES
    label = new AustinColorCamLabel();
    label->setMinimumHeight(200);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->onEnableSavePoints(true);
    this->layout()->addWidget(label);

    // CREATE A ROBOT OBJECT FOR CONTROLLING ROBOT
    object = new AustinColorCamObject(ipAddr, portNum);

    // NOW THAT WE'VE MADE OUR CONNECTIONS, TELL ROBOT OBJECT TO CONNECT OVER SERIAL/TCP
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
AustinColorCamWidget::~AustinColorCamWidget()
{
    if (object) {
        delete object;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
AustinColorCamLabel::AustinColorCamLabel(QWidget *parent) : QLabel(parent), savePointsFlag(false), counter(0)
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
void AustinColorCamLabel::mousePressEvent(QMouseEvent *event)
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
void AustinColorCamLabel::onEnableSavePoints(bool state)
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
void AustinColorCamLabel::onAddPoint(QPoint pt)
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
void AustinColorCamLabel::onAddPoints(QList<QPoint> pts)
{
    for (int n = 0; n < pts.count(); n++) {
        onAddPoint(pts.at(n));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void AustinColorCamLabel::onAddPoints(QVector<QPoint> pts)
{
    for (int n = 0; n < pts.count(); n++) {
        onAddPoint(pts[n]);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void AustinColorCamLabel::onSavePoints()
{
    // SAVE THE SCANNING DATA TO DISK
    QSettings settings;
    QString directory = settings.value("AustinColorCamWidget::lastDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString filename = QFileDialog::getSaveFileName(this, QString("Save tracking data to disk (*.csv)"), directory, QString("*.csv"));
    if (filename.isEmpty() == false) {
        if (filename.toLower().endsWith(QString(".csv")) == false) {
            filename.append(QString(".csv"));
        }
        settings.setValue("AustinColorCamWidget::lastDirectory", QFileInfo(filename).absolutePath());

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
void AustinColorCamLabel::paintEvent(QPaintEvent *)
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
AustinColorCamObject::~AustinColorCamObject()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void AustinColorCamObject::onSendMessage(int message, void *argument)
{
    Q_UNUSED(message);
    Q_UNUSED(argument);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void AustinColorCamObject::onReadyRead()
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
QByteArray AustinColorCamObject::processMessage(QByteArray byteArray)
{
    return (QByteArray());
}
