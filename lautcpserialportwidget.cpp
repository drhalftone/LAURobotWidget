#include "lautcpserialportwidget.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPortServer::LAUTCPSerialPortServer(int num, QObject *parent) : QObject(parent)
{
    // GET A LIST OF ALL POSSIBLE SERIAL PORTS CURRENTLY AVAILABLE
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    for (int n = 0; n < portList.count(); n++) {
        ports << new LAUTCPSerialPort(portList.at(n).portName(), num + n);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPortServer::~LAUTCPSerialPortServer()
{
    while (ports.count() > 0) {
        delete ports.takeFirst();
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPort::LAUTCPSerialPort(QString string, int prtNmbr, QObject *parent) : QTcpServer(parent), connected(false), portNumber(prtNmbr), portString(string), port(NULL), socket(NULL), zeroConf(NULL)
{
    // SET THE SERIAL PORT SETTINGS
    port.setPortName(portString);
    port.setBaudRate(QSerialPort::Baud115200);
    port.setDataBits(QSerialPort::Data8);
    port.setStopBits(QSerialPort::OneStop);
    port.setParity(QSerialPort::NoParity);
    port.setFlowControl(QSerialPort::NoFlowControl);

    // CONNECT THE SERIAL PORT TO THE READY READ SLOT
    connect(&port, SIGNAL(readyRead()), this, SLOT(onReadyReadSerial()));

    // CREATE A ZERO CONF INSTANCE AND ADVERTISE THE SERIAL PORT
    zeroConf = new QZeroConf();
    connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
    connect(zeroConf, SIGNAL(servicePublished()), this, SLOT(onServicePublished()));
    zeroConf->startServicePublish(portString.toUtf8(), "_lautcpserialportserver._tcp", "local", portNumber);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPSerialPort::~LAUTCPSerialPort()
{
    if (port.isOpen()) {
        port.close();
    }
    if (socket) {
        delete socket;
    }
    if (zeroConf) {
        delete zeroConf;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::incomingConnection(qintptr handle)
{
    // OPEN THE SERIAL PORT FOR COMMUNICATION
    if (!port.open(QIODevice::ReadWrite)) {
        emit emitError(QString("Cannot connect to RoboClaw.\n") + port.errorString());
    } else if (!port.isReadable()) {
        emit emitError(QString("Port is not readable!\n") + port.errorString());
    } else {
        // SET THE CONNECTED FLAG HIGH
        connected = true;

        // CALL THE BASE CLASS TO USE ITS DEFAULT METHOD
        QTcpServer::incomingConnection(handle);

        // NOW LET'S CREATE THE SOCKET FOR MESSAGE HANDLING TO THE TRACKER
        socket = this->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyReadTCP()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

        QHostAddress hostAddress = socket->peerAddress();
        clientIPAddress = hostAddress.toString();
        qDebug() << "LAU3DVideoTCPServer :: Accepting incoming connection from " << clientIPAddress;

        // STOP LISTENING FOR NEW CONNECTIONS
        this->close();
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onReadyReadTCP()
{
    ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onReadyReadSerial()
{
    ;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onDisconnected()
{
    // CLOSE THE SERIAL PORT CONNECTION
    port.close();

    // DELETE SOCKET TO CLOSE CONNECTION
    if (socket) {
        socket->deleteLater();
        socket = NULL;
    }
    connected = false;

    // START LISTENING FOR A NEW CONNECTION
    if (!this->isListening()) {
        if (!this->listen(QHostAddress::Any, portNumber)) {
            qDebug() << "LAU3DVideoTCPServer :: Error trying to listen for incoming connections!";
        }
    }
    qDebug() << "LAU3DVideoTCPServer :: Closing connection from " << clientIPAddress;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onServicePublished()
{
    if (!this->listen(QHostAddress::Any, portNumber)) {
        qDebug() << "LAUTCPSerialPort ::" << portString << ":: Error trying to listen for incoming connections!";
    } else {
        qDebug() << "LAUTCPSerialPort ::" << portString << ":: Listening for new connections!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onServiceError(QZeroConf::error_t error)
{
    switch (error) {
        case QZeroConf::noError:
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, portString, QString("Zero Conf Server Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPSerialPort::onTcpError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "LAU3DVideoTCPServer :: Remote host closed error!";
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "LAU3DVideoTCPServer :: Host not found error!";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "LAU3DVideoTCPServer :: Connection refused error!";
            break;
        default:
            qDebug() << "LAU3DVideoTCPClient :: Default error!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUZeroConfClientWidget::LAUZeroConfClientWidget(QString service, QWidget *parent) : QWidget(parent), tcpAddressComboBox(NULL), serviceString(service), zeroConf(NULL)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(0);
    this->setWindowTitle(QString("TCP 3D Video Recorder"));

    QWidget *widget = new QWidget();
    widget->setLayout(new QVBoxLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->addWidget(widget);

    QGroupBox *box = new QGroupBox(QString("Server Address"));
    box->setLayout(new QHBoxLayout());
    box->layout()->setContentsMargins(6, 6, 6, 6);
    widget->layout()->addWidget(box);

    tcpAddressComboBox = new QComboBox();
    tcpAddressComboBox->setMinimumWidth(300);
    connect(tcpAddressComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLineEdit_currentIndexChanged(QString)));

    QLabel *label = new QLabel(QString("Host:"));
    label->setFixedWidth(40);
    box->layout()->addWidget(label);
    box->layout()->addWidget(tcpAddressComboBox);

    // MAKE CONNECTIONS BETWEEN THIS OBJECT AND THE BONJOUR SERVICE OBJECT
    zeroConf = new QZeroConf();
    connect(zeroConf, SIGNAL(serviceAdded(QZeroConfService *)), this, SLOT(onAddService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(serviceRemoved(QZeroConfService *)), this, SLOT(onRemoveService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(serviceUpdated(QZeroConfService *)), this, SLOT(onUpdateService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
    zeroConf->startBrowser(serviceString, QAbstractSocket::IPv4Protocol);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUZeroConfClientWidget::~LAUZeroConfClientWidget()
{
    if (zeroConf) {
        delete zeroConf;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onServiceError(QZeroConf::error_t error)
{
    switch (error) {
        case QZeroConf::noError:
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onAddService(QZeroConfService *item)
{
    tcpAddressComboBox->addItem(QString("%1::%2").arg(item->name).arg(item->port), item->ip.toString());
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onRemoveService(QZeroConfService *item)
{
    tcpAddressComboBox->removeItem(tcpAddressComboBox->findText(QString("%1::%2").arg(item->name).arg(item->port)));
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onUpdateService(QZeroConfService *)
{
    qDebug() << "LAUZeroConfClientWidget :: Update service!";
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onLineEdit_currentIndexChanged(QString string)
{
    QStringList strings = string.split(QString("::"));
    if (strings.count() == 2) {
        ipString = tcpAddressComboBox->currentData().toString();
        portNumber = strings.last().toInt();
        emit emitValidAddress(true);
    } else {
        ipString = QString();
        portNumber = -1;
        emit emitValidAddress(false);
    }
}
