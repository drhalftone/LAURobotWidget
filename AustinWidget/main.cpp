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

    QString Qtopic;
    std::string topic;

    qDebug ("Hit Enter to continue");
    std::cin >> topic;
    Qtopic.fromStdString(topic);
    msg.printMSGS(Qtopic);
    while (1) {
        qDebug ("Enter in the ROS Topic that you would like to view: ");
        std::cin >> topic;
        Qtopic.fromStdString(topic);
        msg.printMSGS(Qtopic);
    }
#endif

    return a.exec();
}
