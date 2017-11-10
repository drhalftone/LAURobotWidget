#include "lautcprosportwidget.h"

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
    if (tpc.isEmpty()) {
        // GET A LIST OF ALL POSSIBLE ROS TOPICS CURRENTLY AVAILABLE
        ros::master::V_TopicInfo topics;
        if (ros::master::getTopics(topics)) {
            for (unsigned int n = 0; n < topics.size(); n++) {
                ports << new LAUTCPROSPort(QString::fromStdString(topics.at(n).name), QString::fromStdString(topics.at(n).datatype), num + n);
            }
        }
    } else {
        // GET A LIST OF ALL POSSIBLE ROS TOPICS CURRENTLY AVAILABLE
        ros::master::V_TopicInfo topics;
        if (ros::master::getTopics(topics)) {
            for (unsigned int n = 0; n < topics.size(); n++) {
                if (QString::fromStdString(topics.at(n).name) == tpc) {
                    ports << new LAUTCPROSPort(QString::fromStdString(topics.at(n).name), QString::fromStdString(topics.at(n).datatype), num + n);
                }
            }
        }
    }
#endif
    // START THE TIMER TO CHECK FOR ROS EVENTS
    if (ports.isEmpty() == false) {
        startTimer(100);
    }
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
LAUTCPROSPort::LAUTCPROSPort(QString tpc, QString dType, int prtNmbr, QObject *parent) : QTcpServer(parent), connected(false), portNumber(prtNmbr), socket(NULL), zeroConf(NULL), topicString(tpc)
{
    qDebug() << tpc << dType;
#ifdef LAU_ROS
    // SET THE SERIAL PORT SETTINGS
    if (node.ok()) {
        subscriber = node.subscribe(topicString.toStdString(), 1000, &LAUTCPROSPort::callbackOdom, this);
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
    // CLOSE THE TCP SOCKET
    if (socket) {
        delete socket;
    }

    // DELETE THE ZERO CONF INSTANCE
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
#else
    Q_UNUSED(error);
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

#ifdef LAU_ROS
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::callbackOdom(const nav_msgs::Odometry::ConstPtr &msg)
{
    qDebug() << "LAUTCPROSPort::callbackOdom(const nav_msgs::Odometry::ConstPtr &msg)";
    if (isConnected()){
        double buffer[7];
        buffer[0] = msg->pose.pose.orientation.w;
        buffer[1] = msg->pose.pose.orientation.x;
        buffer[2] = msg->pose.pose.orientation.y;
        buffer[3] = msg->pose.pose.orientation.z;

        buffer[4] = msg->pose.pose.position.x;
        buffer[5] = msg->pose.pose.position.y;
        buffer[6] = msg->pose.pose.position.z;

        socket->write((char*)&buffer[0], sizeof(double)*7);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::callbackColorCamera(const sensor_msgs::Image::ConstPtr &msg)
/*
 * Encoding of pixels -- channel meaning, ordering, size taken from the list
 * of string in include/sensor_msgs/image_encodings.h
 */
{
    qDebug() << "LAUTCPROSPORT::callbackColorCamera(const sensor_msgs::Image::ConstPtr &msg)\n";

    // Encoding of pixels -- channel meaning, ordering, size taken from the list
    // of string in include/sensor_msgs/image_encodings.h
    if (isConnected()) {
        //std::string encoding = msg->encoding;
        socket->write((char*)&(msg->step), sizeof(int));
        socket->write((char*)&(msg->height), sizeof(int));
        socket->write((char*)&(msg->width), sizeof(int));
        socket->write((char*)&(msg->is_bigendian), sizeof(unsigned char));
        for ( int i = 0; i < msg->data.size(); ++i)
        {
            socket->write((char*)msg->data[i], msg->step * msg->height);
        }
    }
}

void LAUTCPROSPort::callbackPointCloud2(const sensor_msgs::PointCloud2::ConstPtr &msg)
/*
* This message holds a collection of N-dimensional points, which may
* contain additional information such as normals, intensity, etc. The
* point data is stored as a binary blob, its layout described by the
* contents of the "fields" array.
* The point cloud data may be organized 2d (image-like) or 1d
* (unordered). Point clouds organized as 2d images may be produced by
* camera depth sensors such as stereo or time-of-flight.
*/
{
    qDebug() << "LAUTCPROSPort::callbackPointCloud2(const sensor_msgs::PointCloud2::ConstPtr &msg)\n";

    //Time of sensor data acquisition, and the coordinate frame ID (for 3d
    //points).
    if (isConnected()) {
        //ros::Header header = msg->header;

        socket->write((char*)&(msg->header), sizeof(unsigned char));
        socket->write((char*)&(msg->point_step), sizeof(int));
        socket->write((char*)&(msg->row_step), sizeof(int));
        socket->write((char*)&(msg->height), sizeof(int));
        socket->write((char*)&(msg->width), sizeof(int));
        socket->write((char*)&(msg->is_bigendian), sizeof(unsigned char));
        for ( int i = 0; i < msg->data.size(); ++i)
        {
            socket->write((char*)&(msg->data[i]), msg->row_step * msg->height);
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::callbackIMU(const sensor_msgs::Imu::ConstPtr &msg)
/*
* This is a message to hold data from an IMU (Inertial Measurement Unit)
*
* Accelerations should be in m/s^2 (not in g's), and rotational velocity should be in rad/sec
*
* If the covariance of the measurement is known, it should be filled in (if all you know is the
* variance of each measurement, e.g. from the datasheet, just put those along the diagonal)
* A covariance matrix of all zeros will be interpreted as "covariance unknown", and to use the
* data a covariance will have to be assumed or gotten from some other source
*
* If you have no estimate for one of the data elements (e.g. your IMU doesn't produce an orientation
* estimate), please set element 0 of the associated covariance matrix to -1
* If you are interpreting this message, please check for a value of -1 in the first element of each
* covariance matrix, and disregard the associated estimate.
*/
{void callback(const nav_msgs::Odometry::ConstPtr &msg);
    if (isConnected()) {
        qDebug() << "LAUTCPROSPort::callbackIMU(const sensor_msgs::Imu::ConstPtr &msg)\n";

        //ros::Header header = msg->header; TODO

        double buffer[36];
        /* Quaternion Orientation */
        buffer[0] = msg->orientation.x;
        buffer[1] = msg->orientation.y;
        buffer[2] = msg->orientation.z;
        buffer[3] = msg->orientation.w;

        /* Orientation Covariance */
        buffer[4] = msg->orientation_covariance[0];
        buffer[5] = msg->orientation_covariance[1];
        buffer[6] = msg->orientation_covariance[2];
        buffer[7] = msg->orientation_covariance[3];
        buffer[8] = msg->orientation_covariance[4];
        buffer[9] = msg->orientation_covariance[5];
        buffer[10] = msg->orientation_covariance[6];
        buffer[11] = msg->orientation_covariance[7];
        buffer[12] = msg->orientation_covariance[8];

        /* Angular Velocity */
        buffer[13] = msg->angular_velocity.x;
        buffer[14] = msg->angular_velocity.y;
        buffer[15] = msg->angular_velocity.z;

        /* Angular velocity Covariance */
        buffer[16] = msg->angular_velocity_covariance[0];
        buffer[17] = msg->angular_velocity_covariance[1];
        buffer[18] = msg->angular_velocity_covariance[2];
        buffer[19] = msg->angular_velocity_covariance[3];
        buffer[20] = msg->angular_velocity_covariance[4];
        buffer[21] = msg->angular_velocity_covariance[5];
        buffer[22] = msg->angular_velocity_covariance[6];
        buffer[23] = msg->angular_velocity_covariance[7];
        buffer[24] = msg->angular_velocity_covariance[8];

        /* Linear Acceleration */
        buffer[25] = msg->linear_acceleration.x;
        buffer[26] = msg->linear_acceleration.y;
        buffer[27] = msg->linear_acceleration.z;

        /* Linear Acceleration Covariance */
        buffer[28] = msg->linear_acceleration_covariance[0];
        buffer[29] = msg->linear_acceleration_covariance[1];
        buffer[30] = msg->linear_acceleration_covariance[2];
        buffer[31] = msg->linear_acceleration_covariance[3];
        buffer[32] = msg->linear_acceleration_covariance[4];
        buffer[33] = msg->linear_acceleration_covariance[5];
        buffer[34] = msg->linear_acceleration_covariance[6];
        buffer[35] = msg->linear_acceleration_covariance[7];
        buffer[36] = msg->linear_acceleration_covariance[8];


        socket->write((char*)&buffer[0], sizeof(double)*37);
    }
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUTCPROSPort::callbackLog(const rosgraph_msgs::Log::ConstPtr & log_msg)
/* This is a message to get INFO messaged that ROS procduces */
{
    if(isConnected()) {
        QString msg_buffer[5];

        /*Determine which level of Log.msg we are getting */
        int severity_level = log_msg->INFO;
        switch (severity_level)
        {
        case 1 : msg_buffer[0] = "DEBUG"; break;
        case 2 : msg_buffer[0] = "INFO"; break;
        case 4 : msg_buffer[0] = "WARN"; break;
        case 8 : msg_buffer[0] = "ERROR"; break;
        case 16 : msg_buffer[0] = "FATAL/CRITICAL"; break;
        default : return; break;
        }
        msg_buffer[1] = QString::fromStdString(log_msg->name);
        msg_buffer[2] = QString::fromStdString(log_msg->msg);
        msg_buffer[3] = QString::fromStdString(log_msg->file);
        msg_buffer[4] = QString::fromStdString(log_msg->function);
        //msg_buffer[5] = QString::fromStdString(log_msg->header);

        //socket->write((char*)&buffer[0], sizeof(double)*31)
    }
}

#endif
