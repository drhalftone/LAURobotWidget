#ifndef LAURPLIDARWIDGET_H
#define LAURPLIDARWIDGET_H

#ifndef LAU_SERVER
#include <QMenu>
#include <QWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QInputDialog>
#endif

#include <QString>
#include <QTime>
#include <QList>
#include <QtCore>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QPainter>
#include <QVector4D>
#include <QSettings>
#include <QSerialPort>
#include <QStandardPaths>
#include <QSerialPortInfo>
#include <QMouseEvent>

#include "lautcpserialportwidget.h"

#define LAURPLIDAR_SERVERIDSTRING   "_lautcprplidarserver._tcp"
#define LAURPLIDAR_FIXED_BYTE     0xA5
#define LAURPLIDAR_STOP           0x25
#define LAURPLIDAR_RESET          0x40
#define LAURPLIDAR_SCAN           0x20
#define LAURPLIDAR_EXPRESS_SCAN   0x82
#define LAURPLIDAR_FORCE_SCAN     0x21
#define LAURPLIDAR_GET_INFO       0x50
#define LAURPLIDAR_GET_HEALTH     0x52
#define LAURPLIDAR_GET_SAMPLERATE 0x59

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURPLidarServer : public LAUTCPSerialPortServer
{
    Q_OBJECT

public:
    explicit LAURPLidarServer(int num = LAUTCPSERIALPORTSERVERPORTNUMER, unsigned short identifier = 0xFFFF, QObject *parent = 0) : LAUTCPSerialPortServer(num , identifier, QString(LAURPLIDAR_SERVERIDSTRING), parent) { ; }
};

#ifndef LAU_SERVER
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURPLidarObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    enum ScanState { StateNotScanning, StateExpressScan, StateScan };

    LAURPLidarObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent), scanState(StateNotScanning), scan(32)
    {
        record.setFileName(QString("/tmp/record.txt"));
        record.open(QIODevice::WriteOnly);
    }

    LAURPLidarObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, QString(LAURPLIDAR_SERVERIDSTRING), parent), scanState(StateNotScanning), scan(32)
    {
        record.setFileName(QString("/tmp/record.txt"));
        record.open(QIODevice::WriteOnly);
    }

    ~LAURPLidarObject();

public slots:
    void onReadyRead();
    void onSendMessage(int message, void *argument);
    void onError(QString error)
    {
        emit emitError(error);
    }

    void onScan();
    void onStop();
    void onReset();
    void onGetInfo();
    void onGetHealth();
    void onForceScan();
    void onExpressScan();
    void onGetSampleRate();
    void onInitiateScanning();

private:
    typedef struct {
        int message;
        void *argument;
    } Packet;

    QByteArray appendCRC(QByteArray byteArray);
    QByteArray processMessage(QByteArray byteArray);
    int decodeMessageHeader(QByteArray byteArray);

    QList<Packet> messageList;
    ScanState scanState;
    int modelNumber;
    int versionMinor;
    int versionMajor;
    int hardware;
    double angleA, angleB;
    QByteArray serialNumber;
    QVector<QPoint> scan;

    QPoint getPoint(double Aa, double Ab, double dA, double D, int k);

    QFile record;

private slots:
    void onSendNextMessage();

signals:
    void emitError(QString string);
    void emitPoint(QPoint pt);
    void emitScan(QVector<QPoint> pts);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURPLidarLabel : public QLabel
{
    Q_OBJECT

public:
    LAURPLidarLabel(QWidget *parent = 0);

public slots:
    void onAddPoint(QPoint pt);
    void onAddPoints(QList<QPoint> pts);
    void onAddPoints(QVector<QPoint> pts);
    void onSavePoints();
    void onEnableSavePoints(bool state);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    bool savePointsFlag;
    QList<QPoint> points;
    QPoint topLeft;
    QPoint bottomRight;
    QMenu *contextMenu;
    int counter;
    QTime time;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURPLidarWidget : public QWidget
{
    Q_OBJECT

public:
    LAURPLidarWidget(QString portString = QString(), QWidget *parent = 0);
    LAURPLidarWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~LAURPLidarWidget();

    bool isValid()
    {
        return (object && object->isValid());
    }

protected:
    void showEvent(QShowEvent *);

private:
    LAURPLidarObject *object;
    LAURPLidarObject *robot;
    LAURPLidarLabel *label;
    LAURPLidarLabel *robolabel;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURPLidarDialog : public QDialog
{
    Q_OBJECT

public:
    LAURPLidarDialog(QString portString = QString(), QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("ENCODER LAURPLidar Dialog"));

        widget = new LAURPLidarWidget(portString);
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    LAURPLidarDialog(QString ipAddr, int portNum, QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAURPLidar Dialog"));

        widget = new LAURPLidarWidget(ipAddr, portNum);
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    bool isValid()
    {
        return (widget->isValid());
    }

public slots:

private:
    LAURPLidarWidget *widget;
};
#endif
#endif // LAURPLIDARWIDGET_H
