#include "widget.h"
#include "AustinRosWidget.h"
#include <QApplication>


#ifdef Austin_ROS
#include "AustinRosWidget.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Austin_ROS
    ros::init(argc, argv, "AustinRosWidget");
    ros::start();

    Austin_ROS_MSGS msg;
    std::string topic;
    QString Qtopic;
    //ros::spinOnce();
    while (1) {
        //ros::spinOnce();
        qDebug ("Enter in the ROS Topic that you would like to view, "
                "Enter 'topics' to view all available topics that you can subscribe to: ");
        std::getline(std::cin,topic);       
        Qtopic = QString::fromStdString(topic);
        msg.printMSGS(Qtopic);
        //ros::spinOnce();
    }
#endif

    return a.exec();
}
