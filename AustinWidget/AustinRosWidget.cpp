#include "widget.h"
#include "AustinRosWidget.h"

#ifdef Austin_ROS
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::callbackLog(const rosgraph_msgs::Log::ConstPtr & log_msg)
{
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
    subscriber.shutdown();

}

/******************************************************************************/
void Austin_ROS_MSGS::callbackOdometry(const nav_msgs::Odometry::ConstPtr &msg)
{
    //if (isConnected()) {
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
        //socket->write((char *)&buffer[0], sizeof(double) * 7);
    //}
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void Austin_ROS_MSGS::printMSGS(QString topic)
{
    topicString = topic;
    qDebug() << "Printing: " << topicString << endl;

    //ros::spinOnce();
    if (node.ok()) {
        if (topicString == QString("rosout")){
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackLog, this);
            ros::spinOnce();
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackLog, this);
            ros::spinOnce();
        }
        else if (topicString == QString("odometry")){
            subscriber = node.subscribe(topicString.toStdString(), 1000, &Austin_ROS_MSGS::callbackOdometry, this);
            ros::spinOnce();
        }
    } else {
        qDebug() << "ERROR: Node not ok.";
      }
}
#endif
