#ifndef LAUTCPSERIALPORTWIDGET_H
#define LAUTCPSERIALPORTWIDGET_H

#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDialogButtonBox>

#include <qzeroconf.h>

#define LAUTCPSERIALPORTSERVERPORTNUMER  11364

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPSerialPort : public QTcpServer
{
    Q_OBJECT

public:
    explicit LAUTCPSerialPort(QString string, int prtNmbr = LAUTCPSERIALPORTSERVERPORTNUMER, QObject *parent = 0);
    ~LAUTCPSerialPort();

    bool isConnected() const
    {
        return (connected);
    }
    QString ipAddress() const
    {
        return (clientIPAddress);
    }
    int localPort() const
    {
        return (portNumber);
    }

protected:
    void incomingConnection(qintptr handle);

private slots:
    void onDisconnected();
    void onReadyReadTCP();
    void onReadyReadSerial();
    void onServicePublished();
    void onServiceError(QZeroConf::error_t error);
    void onTcpError(QAbstractSocket::SocketError error);

private:
    bool connected;            // FLAG TO INDICATE WE ARE CONNECTED TO A CLIENT
    int portNumber;            // PORT NUMBER
    QString portString;        // PORT STRING
    QSerialPort port;          // INSTANCE OF THE SERIAL PORT
    QTcpSocket *socket;        // TCP SOCKET TO HOLD THE INCOMING CONNECTION
    QZeroConf *zeroConf;       // ZERO CONF TO ADVERTISE PORT
    QString clientIPAddress;   // IP ADDRESS OF CLIENT, IF THERE IS ONE

signals:
    void emitError(QString string);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUTCPSerialPortServer : public QObject
{
    Q_OBJECT

public:
    explicit LAUTCPSerialPortServer(int num = LAUTCPSERIALPORTSERVERPORTNUMER, QObject *parent = 0);
    ~LAUTCPSerialPortServer();

    int channels() const
    {
        return (ports.count());
    }
    bool isConnected(int n) const
    {
        return (ports.at(n)->isConnected());
    }
    QString ipAddress(int n) const
    {
        return (ports.at(n)->ipAddress());
    }
    int localPort(int n) const
    {
        return (ports.at(n)->localPort());
    }

private:
    QList<LAUTCPSerialPort *> ports;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUZeroConfClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUZeroConfClientWidget(QString service = QString("_qtzeroconf_test._tcp"), QWidget *parent = 0);
    ~LAUZeroConfClientWidget();

    QString address() const
    {
        return (ipString);
    }

    int port() const
    {
        return (portNumber);
    }

public slots:
    void onLineEdit_currentIndexChanged(QString string);

private:
    QComboBox *tcpAddressComboBox;
    QString ipString, serviceString;
    QZeroConf *zeroConf;
    int portNumber;

private slots:
    void onServiceError(QZeroConf::error_t error);
    void onRemoveService(QZeroConfService *item);
    void onUpdateService(QZeroConfService *);
    void onAddService(QZeroConfService *item);

signals:
    void emitValidAddress(bool state);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUZeroConfClientDialog : public QDialog
{
    Q_OBJECT

public:
    LAUZeroConfClientDialog(QString serviceString = QString("_qtzeroconf_test._tcp"), QWidget *parent = 0) : QDialog(parent)
    {
        this->setWindowTitle(QString("TCP Server Address"));
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->setSpacing(6);

        widget = new LAUZeroConfClientWidget(serviceString, this);
        this->layout()->addWidget(widget);

        QPushButton *button = new QPushButton(QString("Connect"));
        button->setEnabled(false);
        connect(button, SIGNAL(clicked()), this, SLOT(accept()));
        connect(widget, SIGNAL(emitValidAddress(bool)), button, SLOT(setEnabled(bool)));

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
        buttonBox->addButton(button, QDialogButtonBox::AcceptRole);
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }
    ~LAUZeroConfClientDialog() { ; }

    QString address() const
    {
        return (ipString);
    }

    int port() const
    {
        return (portNumber);
    }

protected:
    void showEvent(QShowEvent *)
    {
        this->setFixedSize(this->size());
    }
    void accept()
    {
        ipString = widget->address();
        portNumber = widget->port();
        QDialog::accept();
    }

private:
    LAUZeroConfClientWidget *widget;
    QString ipString, serviceString;
    int portNumber;
};

#endif // LAUTCPSERIALPORTWIDGET_H
