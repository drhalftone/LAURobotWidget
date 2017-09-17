#ifndef LAUODOMWIDGET_H
#define LAUODOMWIDGET_H

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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUOdomObject : public LAUTCPSerialPortClient
{
    Q_OBJECT

public:
    LAUOdomObject(QString portString, QObject *parent = 0) : LAUTCPSerialPortClient(portString, parent) { ; }
    LAUOdomObject(QString ipAddr, int portNum, QObject *parent = 0) : LAUTCPSerialPortClient(ipAddr, portNum, parent) { ; }
    ~LAUOdomObject();

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
class LAUOdomLabel : public QLabel
{
    Q_OBJECT

public:
    LAUOdomLabel(QWidget *parent = 0);

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
class LAUOdomWidget : public QWidget
{
    Q_OBJECT

public:
    LAUOdomWidget(QString portString = QString(), QWidget *parent = 0);
    LAUOdomWidget(QString ipAddr, int portNum, QWidget *parent = 0);
    ~LAUOdomWidget();

    bool isValid()
    {
        return (object && object->isValid());
    }

private:
    LAUOdomObject *object;
    LAUOdomLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUOdomDialog : public QDialog
{
    Q_OBJECT

public:
    LAUOdomDialog(QString portString = QString(), QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAUOdom Dialog"));

        widget = new LAUOdomWidget(portString);
        this->layout()->addWidget(widget);

        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(box);
    }

    LAUOdomDialog(QString ipAddr, int portNum, QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("LAUOdom Dialog"));

        widget = new LAUOdomWidget(ipAddr, portNum);
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
    LAUOdomWidget *widget;
};

#endif // LAUODOMWIDGET_H
