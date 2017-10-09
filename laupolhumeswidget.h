#ifndef LAUPOLHUMESWIDGET_H
#define LAUPOLHUMESWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QGroupBox>
#include <QQuaternion>
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

#define LAUPOLHEMUS_ALIGNMENT_REFERENCE_FRAME     0x41
#define LAUPOLHEMUS_BORESIGHT                     0x42
#define LAUPOLHEMUS_CONTINUOUS_PRINT_OUTPUT       0x43
#define LAUPOLHEMUS_SET_UNITS                     0x55

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUPolhemusObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    LAUPolhemusObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent) { ; }
    LAUPolhemusObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, parent) { ; }
    ~LAUPolhemusObject();

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
    void emitOdometry(QQuaternion pose, QVector3D position);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUPolhemusLabel : public QLabel
{
    Q_OBJECT

public:
    LAUPolhemusLabel(QWidget *parent = 0);

public slots:
    void onSavePoints();
    void onEnableSavePoints(bool state);
    void onUpdateOdometry(QQuaternion pose, QVector3D position);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);

private:
    bool savePointsFlag;
    QList<QQuaternion> poses;
    QList<QVector3D> points;
    QVector3D topLeft;
    QVector3D bottomRight;
    QMenu *contextMenu;
    int counter;
    QTime time;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUPolhemusWidget : public QWidget
{
    Q_OBJECT

public:
    LAUPolhemusWidget(QString portString = QString(), QWidget *parent = 0);
    LAUPolhemusWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~LAUPolhemusWidget();

    bool isValid()
    {
        return (object && object->isValid());
    }

protected:
    void showEvent(QShowEvent *);

private:
    LAUPolhemusObject *object;
    LAUPolhemusLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUPolhemusDialog : public QDialog
{
    Q_OBJECT

public:
    LAUPolhemusDialog(QString portString = QString(), QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAUPolhemus Dialog"));

        widget = new LAUPolhemusWidget(portString);
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    LAUPolhemusDialog(QString ipAddr, int portNum, QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAUPolhemus Dialog"));

        widget = new LAUPolhemusWidget(ipAddr, portNum);
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
    LAUPolhemusWidget *widget;
};
#endif // LAUPOLHUMESWIDGET_H
