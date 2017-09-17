#include "lautcprosportwidget.h"

#ifdef LAU_ROS
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void chatterCallback(const nav_msgs::Odometry::ConstPtr &msg)
{
    qDebug() << msg->pose.pose.orientation.x << msg->pose.pose.orientation.y << msg->pose.pose.orientation.z << msg->pose.pose.orientation.w;
}
#endif

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPortServer::LAUTCPROSPortServer(int num, QString tpc, QObject *parent) : QObject(parent)
{
    // DROP IN DEFAULT PORT NUMBER IF USER SUPPLIED VALUE IS NEGATIVE
    if (num < 0) {
        num = LAUTCPROSPORTSERVERPORTNUMER;
    }

#ifdef LAU_ROS
    // INITIALIZE AND START THE ROS ENGINE IF IT HASN'T ALREADY BEEN STARTED
    if (ros::isInitialized() == false) {
        ros::init(argc, argv, "LAUTCPROSPortServer");
        ros::start();
    }

    // LAUNCH A TIMER TO TRIGGER ROS MESSAGE HANDLING
    startTimer(100);

    // GET A LIST OF ALL POSSIBLE ROS TOPICS CURRENTLY AVAILABLE
    ros::master::V_TopicInfo topics;
    if (ros::master::getTopics(topics)) {
        for (int n = 0; n < topics.size(); n++) {
            ports << new LAUTCPROSPort(QString::fromStdString(topics.at(n).name), num + n);
        }
    }
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPortServer::~LAUTCPROSPortServer()
{
    while (ports.count() > 0) {
        delete ports.takeFirst();
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPort::LAUTCPROSPort(QString string, int prtNmbr, QObject *parent) : QTcpServer(parent), connected(false), portNumber(prtNmbr), socket(NULL), zeroConf(NULL), topicString(string)
{
#ifdef LAU_ROS
    // SET THE SERIAL PORT SETTINGS
    if (node.ok()) {
        subscriber = node.subscribe(topicString.toStdString(), 1000, chatterCallback);
    } else {
        qDebug() << "ERROR: Node not ok.";
    }
#endif
    // CREATE A ZERO CONF INSTANCE AND ADVERTISE THE SERIAL PORT
    zeroConf = new QZeroConf();
    connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
    connect(zeroConf, SIGNAL(servicePublished()), this, SLOT(onServicePublished()));
    zeroConf->startServicePublish(topicString.toUtf8(), "_LAUTCPROSPortserver._tcp", "local", portNumber);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUTCPROSPort::~LAUTCPROSPort()
{
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
void LAUTCPROSPort::incomingConnection(qintptr handle)
{
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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onReadyReadTCP()
{
    // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
    QByteArray byteArray = socket->readAll();
    if (byteArray.isEmpty() == false) {
        qDebug() << "LAUTCPROSPort" << byteArray;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onReadyReadROS()
{
    // READ THE CURRENTLY AVAILABLE BYTES FROM THE PORT
    QByteArray byteArray;
    if (byteArray.isEmpty() == false) {
        // SEND THE BYTES TO THE TCP CLIENT
        socket->write(byteArray);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onDisconnected()
{
    // DELETE SOCKET TO CLOSE CONNECTION
    if (socket) {
        socket->deleteLater();
        socket = NULL;
    }
    connected = false;

    // START LISTENING FOR A NEW CONNECTION
    if (!this->isListening()) {
        if (!this->listen(QHostAddress::Any, portNumber)) {
            qDebug() << "LAUTCPROSPort :: Error trying to listen for incoming connections!";
        }
    }
    qDebug() << "LAUTCPROSPort :: Closing connection from " << clientIPAddress;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onServicePublished()
{
    if (!this->listen(QHostAddress::Any, portNumber)) {
        qDebug() << "LAUTCPROSPort ::" << topicString << ":: Error trying to listen for incoming connections!";
    } else {
        qDebug() << "LAUTCPROSPort ::" << topicString << ":: Listening for new connections!";
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onServiceError(QZeroConf::error_t error)
{
#ifdef LAU_CLIENT
    switch (error) {
        case QZeroConf::noError:
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, topicString, QString("Zero Conf Server Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, topicString, QString("Zero Conf Server Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, topicString, QString("Zero Conf Server Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
#endif
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::onTcpError(QAbstractSocket::SocketError error)
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
