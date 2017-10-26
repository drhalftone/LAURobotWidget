#include "widget.h"
#include "AustinRosWidget.h"

#ifdef Austin_ROS

Austin_ROS_MSGS::Austin_ROS_MSGS()
{
    qDebug() << "IN CONSTRUCTOR";
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
void Austin_ROS_MSGS::callbackOdometry(const nav_msgs::Odometry::ConstPtr &msg)
{
        qDebug() << "Printing Odometry\n";
        double buffer[7];
        buffer[0] = msg->pose.pose.orientation.w;
        buffer[1] = msg->pose.pose.orientation.x;
        buffer[2] = msg->pose.pose.orientation.y;
        buffer[3] = msg->pose.pose.orientation.z;

        buffer[4] = msg->pose.pose.position.x;
        buffer[5] = msg->pose.pose.position.y;
        buffer[6] = msg->pose.pose.position.z;

        for (int i =0; i < 7; ++i) {
            qDebug() << buffer[i] << endl;
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
    std::string encoding = msg->encoding;
    uint32_t step = msg->step; // Full row length in bytes
    uint32_t height = msg->height; //image height, number of rows
    uint32_t width = msg->width; //omage width, number of columns
    uint8_t is_bigendian = msg->is_bigendian; //Is this data big endian?
    uint32_t matrix_size = step*height;
    std::vector<uint8_t> image_data(matrix_size);

    for (uint32_t i = 0; i < matrix_size; ++i)
    {
        image_data[i] = msg->data[i];
    }
    /* PROCESS CAMERA DATA */

    subscriber.shutdown();
    ros::shutdown();


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
        else if (topicString == QString("odometry")){
            qDebug() << "subscribing to odometry\n";
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackOdometry, this);
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
    } else {
        qDebug() << "ERROR: Node not ok.";
      }
}
#endif
