#ifndef AUSTINCOLORCAMWIDGET_H
#define AUSTINCOLORCAMWIDGET_H

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

#include "lautcpserialportwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class AustinColorCamObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    AustinColorCamObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent) { ; }
    AustinColorCamObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, parent) { ; }
    ~AustinColorCamObject();

public slots:
    void onReadyRead();
    void onSendMessage(int message, void *argument);
    void onError(QString error)
    {
        emit emitError(error);
    }

private:
    QByteArray processMessage(QByteArray byteArray);

signals:
    void emitError(QString string);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class AustinColorCamLabel : public QLabel
{
    Q_OBJECT

public:
    AustinColorCamLabel(QWidget *parent = 0);

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
class AustinColorCamWidget : public QWidget
{
    Q_OBJECT

public:
    AustinColorCamWidget(QString portString = QString(), QWidget *parent = 0);
    AustinColorCamWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~AustinColorCamWidget();

    bool isValid()
    {
        return (object && object->isValid());
    }

private:
    AustinColorCamObject *object;
    AustinColorCamLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class AustinColorCamDialog : public QDialog
{
    Q_OBJECT

public:
    AustinColorCamDialog(QString portString = QString(), QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("AustinColorCam Dialog"));

        widget = new AustinColorCamWidget(portString);
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    AustinColorCamDialog(QString ipAddr, int portNum, QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAUOdom Dialog"));

        widget = new AustinColorCamWidget(ipAddr, portNum);
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
    AustinColorCamWidget *widget;
};

#endif // AUSTINCOLORCAMWIDGET_H
