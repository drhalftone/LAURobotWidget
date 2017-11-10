#include "widget.h"
#include "AustinRosWidget.h"

#ifdef Austin_ROS

Austin_ROS_MSGS::Austin_ROS_MSGS()
{
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::callbackLog(const rosgraph_msgs::Log::ConstPtr & log_msg)
{
    if(topicString == "topics") {
        std::vector<std::string> topicList;
        //topicList.push_back(log_msg->topics);
        //topicList.swap(log_msg->topics);
        for (int i = 0; i < log_msg->topics.size(); ++i) {
            qDebug() << QString::fromStdString(log_msg->topics[i]);
            //qDebug() << QString::fromStdString(log_msg->topics[1]);
        }
    }
    else {
        QString msg_buffer[10];

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

        /*Grab Name*/
        msg_buffer[1] = QString::fromStdString(log_msg->name);

        /*Grab Message*/
        msg_buffer[2] = QString::fromStdString(log_msg->msg);

        /*Grab File*/
        msg_buffer[3] = QString::fromStdString(log_msg->file);

        /*Grab Function*/
        msg_buffer[4] = QString::fromStdString(log_msg->function);

        qDebug() << "Message Level: " << msg_buffer[0];
        qDebug() << "Name: " << msg_buffer[1];
        qDebug() << "Message: " << msg_buffer[2];
        qDebug() << "File: " << msg_buffer[3];
        qDebug() << "Function: " << msg_buffer[4];
        qDebug() << "";
    }
    subscriber.shutdown();
    ros::shutdown();

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::callbackImuData(const sensor_msgs::Imu::ConstPtr &msg)
/*
# This is a message to hold data from an IMU (Inertial Measurement Unit)
#
# Accelerations should be in m/s^2 (not in g's), and rotational velocity should be in rad/sec
#
# If the covariance of the measurement is known, it should be filled in (if all you know is the
# variance of each measurement, e.g. from the datasheet, just put those along the diagonal)
# A covariance matrix of all zeros will be interpreted as "covariance unknown", and to use the
# data a covariance will have to be assumed or gotten from some other source
#
# If you have no estimate for one of the data elements (e.g. your IMU doesn't produce an orientation
# estimate), please set element 0 of the associated covariance matrix to -1
# If you are interpreting this message, please check for a value of -1 in the first element of each
# covariance matrix, and disregard the associated estimate.
*/
{
    qDebug() << "Processing IMU data\n";

    /* Quaternion Orientation */
    double orientation_x = msg->orientation.x;
    double orientation_y = msg->orientation.y;
    double orientation_z = msg->orientation.z;
    double orientation_w = msg->orientation.w;
    /* Orientation Covariance */
    double orientation_covariance[9];
    for (int i = 0; i < 9; ++i) {
        if (-1 == msg->orientation_covariance[0]) {
            orientation_covariance[i] = 0;
        }
        else {
            orientation_covariance[i] = msg->orientation_covariance[i];
        }
    }

    /* Angular Velocity */
    double angular_velocity_x = msg->angular_velocity.x;
    double angular_velocity_y = msg->angular_velocity.y;
    double angular_velocity_z = msg->angular_velocity.z;
    /* Angular velocity Covariance */
    double angular_velocity_covariance[9];
    for (int k = 0; k < 0; ++k) {
        if (-1 == msg->angular_velocity_covariance[0]) {
            angular_velocity_covariance[k] = 0;
        }
        else {
            angular_velocity_covariance[k] = msg->angular_velocity_covariance[k];
        }
    }

    /* Linear Acceleration */
    double linear_acceleration_x = msg->linear_acceleration.x;
    double linear_acceleration_y = msg->linear_acceleration.y;
    double linear_acceleration_z = msg->linear_acceleration.z;
    /* Linear Acceleration Covariance */
    double linear_acceleration_covariance[9];
    for (int j = 0; j < 9; ++j) {
        if (-1 == msg->linear_acceleration_covariance[0]) {
            linear_acceleration_covariance[j] = 0;
        }
        else {
            linear_acceleration_covariance[j] = msg->linear_acceleration_covariance[j];
        }
    }
    subscriber.shutdown();
    ros::shutdown();
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::callbackColorCamera(const sensor_msgs::Image::ConstPtr &msg)
{
    qDebug() << "Processing Color Camera Data\n";

    // Encoding of pixels -- channel meaning, ordering, size taken from the list
    // of string in include/sensor_msgs/image_encodings.h
    //std::string encoding = msg->encoding;
    socket->write((char*)&(msg->step), sizeof(int));
    socket->write((char*)&(msg->height), sizeof(int));
    socket->write((char*)&(msg->width), sizeof(int));
    socket->write((char*)&(msg->is_bigendian), sizeof(unsigned char));
    socket->write((char*)msg->data, msg->step * msg->height);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*void Austin_ROS_MSGS::callbackOdometry(const nav_msgs::Odometry::ConstPtr &msg)
{
    qDebug() << "Odometry";
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
}*/

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void callbackPointCloud2(const sensor_msgs::PointCloud2::ConstPtr &msg)
/*
# This message holds a collection of N-dimensional points, which may
# contain additional information such as normals, intensity, etc. The
# point data is stored as a binary blob, its layout described by the
# contents of the "fields" array.

# The point cloud data may be organized 2d (image-like) or 1d
# (unordered). Point clouds organized as 2d images may be produced by
# camera depth sensors such as stereo or time-of-flight.
*/
{
    qDebug() << "Processing Point Cloud Data\n";

    //Time of sensor data acquisition, and the coordinate frame ID (for 3d
    //points).
    ros::Header header = msg->header;

    //2D structure of the point cloud. If the cloud is unordered, height is
    //1 and width is the length of the point cloud.
    uint32_t height = msg->height;
    uint32_t width = msg->width;

    //Describes the channels and their layout in the binary data blob.

    bool is_bigendian = msg->is_bigendian; //Is this data bigendian?
    uint32_t point_step = msg->point_step; //Length of a point in bytes
    uint32_t row_step = msg->row_step; //Length of a row in bytes

    int matrix_size = row_step * height;
    std::vector<uint8_t> data(matrix_size);

    for (int i = 0; i < matrix_size; ++i) {
        data[i] = msg->data[i];
    }

    bool is_dense = msg->is_dense; //true if there are no invalid points


}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::printMSGS(QString topic)
{
    topicString = topic;

    if (node.ok()) {
        qDebug() << "NODE IS OK";
        if (topicString == QString("rosout")){
            qDebug() << "subscribing to rosout\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackLog, this);
            ros::spin();
        }
        else if (topicString == QString("camera/imu/data_raw")){
            qDebug() << "subscribing to imu data\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackImuData, this);
            ros::spin();
        }
        else if (topicString == QString("topics")){
            qDebug() << "subscribing to topics\n";
            subscriber = node.subscribe("rosout", 1000, &Austin_ROS_MSGS::callbackLog, this);
            ros::spin();
        }
        else if (topicString == QString("camera/color/image_raw")){
            qDebug() << "subscribing to camera/color/image_raw\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackColorCamera, this);
            ros::spin();
        }
        else if (topicString == QString("odometry")) {
            qDebug() << "subscribing to odometry\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackOdometry, this);
            ros::spin();
        }
        else if (topicString == QString("camera/depth/points")) {
            qDebug() << "subscribing to camera/depth/points\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackPointCloud2, this);
            ros::spin();
        }
    } else {
        qDebug() << "ERROR: Node not ok.";
    }
}
#endif
