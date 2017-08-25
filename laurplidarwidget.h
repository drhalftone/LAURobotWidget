#ifndef LAURPLIDARWIDGET_H
#define LAURPLIDARWIDGET_H

#include <QTime>
#include <QList>
#include <QMenu>
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
    LAURPLidarObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent) { ; }
    LAURPLidarObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, parent) { ; }

public slots:
    void onReadyRead();
    void onSendMessage(int message, void *argument = NULL);
    void onError(QString error) { emit emitError(error); }

private:
    QByteArray appendCRC(QByteArray byteArray);
    QByteArray processMessage(QByteArray byteArray);
    int decodeMessageHeader(QByteArray byteArray);

    int modelNumber;
    int versionMinor;
    int versionMajor;
    int hardware;
    QByteArray serialNumber;

signals:
    void emitError(QString string);
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
    void onSavePoints();
    void onEnableSavePoints(bool state);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    bool savePointsFlag;
    QList<QPoint> pts;
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
    LAURPLidarWidget(QWidget *parent = 0);
    ~LAURPLidarWidget();

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
    LAURPLidarDialog(QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAURPLidar Dialog"));

        widget = new LAURPLidarWidget();
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    //bool isValid() { return(widget->isValid()); }

public slots:

private:
    LAURPLidarWidget *widget;
};

#endif // LAURPLIDARWIDGET_H
