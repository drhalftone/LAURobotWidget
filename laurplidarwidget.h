#ifndef LAURPLIDARWIDGET_H
#define LAURPLIDARWIDGET_H

#include <QTime>
#include <QList>
#include <QMenu>
#include <QtCore>
#include <QDebug>
#include <QWidget>
#include <QThread>
#include <QPainter>
#include <QVector4D>
#include <QSettings>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSerialPort>
#include <QFileDialog>
#include <QApplication>
#include <QInputDialog>
#include <QStandardPaths>
#include <QSerialPortInfo>

#include "lautcpserialportwidget.h"

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
class LAURPLidarObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    enum ScanState { StateNotScanning, StateExpressScan, StateScan };

    LAURPLidarObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent), scanState(StateNotScanning), scan(32) { ; }
    LAURPLidarObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, parent), scanState(StateNotScanning), scan(32) { ; }
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
    QByteArray serialNumber;
    QVector<QPoint> scan;

    void sendNextMessage();
    QPoint getPoint(int A, int dA, int D);

signals:
    void emitError(QString string);
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
    LAURPLidarLabel *label;
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
        this->setWindowTitle(QString("LAURPLidar Dialog"));

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

#endif // LAURPLIDARWIDGET_H
