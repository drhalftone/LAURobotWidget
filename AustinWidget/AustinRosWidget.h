#ifndef AUSTINROSWIDGET_H
#define AUSTINROSWIDGET_H

#include <QDebug>
#include <iostream>
#include <string>
#include <vector>
#include <QInputDialog>
#include <QObject>
#include <QTimerEvent>
#include <QQuaternion>
#include <QTextStream>

#ifdef Austin_ROS
#include <ros/ros.h>
#include <ros/master.h>
#include <ros/spinner.h>
#include <rosgraph_msgs/Log.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>
#endif

class Austin_ROS_MSGS : public QObject
{
    Q_OBJECT

public:
    explicit Austin_ROS_MSGS();
    //explicit Austin_ROS_MSGS(QString tpc, QString dType, QObject *parent = 0);
    //~Austin_ROS_MSGS();

    QString topic() const
    {
        return (topicString);
    }


#ifdef Austin_ROS
    /* ROS TOPIC  INFO GETTER FUNCTIONS */
    void callbackLog(const rosgraph_msgs::Log::ConstPtr &log_msg);
    void callbackColorCamera(const sensor_msgs::Image::ConstPtr &msg);
    void callbackImuData(const sensor_msgs::Imu::ConstPtr &msg);
    void callbackOdometry(const nav_msgs::Odometry::ConstPtr &msg);
    void callbackPointCloud2(const sensor_msgs::PointCloud2::ConstPtr &msg);

    void printMSGS(QString topic);


#else
    bool isValid() const
    {
        return (false);
    }
#endif

protected:
    void timerEvent(QTimerEvent *)
    {
#ifdef Austin_ROS
        ros::spinOnce();
#endif
    }

private slots:
    //void onReadyReadROS();
    //void onServiceError(QZeroConf::error_t error);

private:

QString dataTypeString;
QString topicString;  //TOPIC STRING THAT WE ARE LISTENING TO

ros::NodeHandle node;  //HANDLE TO ROS NODE INSTANCE
ros::Subscriber subscriber;  //HANDLE TO ROS SUBSCRIBER INSTANCE


signals:
    void emitError(QString string);

};

#endif // AUSTINROSWIDGET_H
