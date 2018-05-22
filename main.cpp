/**************************************************************************************************
    Copyright (C) 2017 Dr. Daniel L. Lau
    This file is part of LAURobotWidget.

    LAURobotWidget is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LAURobotWidget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LAURobotWidget.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************************************/

/* NOTES: -MM
 *
 * For Palette Motor Interactions, see laurobotwidget.cpp and laurobotwidget.h
 *
 * For Palette setup and config, see laupalettewidget.cpp and laupalettewidget.h
 *
 * For RPLidar setup and config, see laurplidarwidget.cpp and laurplidarwidget.h
 *
 * For ROS setup and config, see lautcprosportwidget.cpp and lautcprosportwidget.h
 *
 * For TCP Serial connection and settings, see lautcpserialportwidget.cpp and lautcpserialportwidget.h
 *
 * lauzeroconfwidget.cpp and lauzeroconfwidget.h should not be modified right now -MM
 *
 */


#ifdef LAU_CLIENT
#include <QApplication>

#include "laukeyencewidget.h"
#include "laurplidarwidget.h"
#include "lauodomwidget.h"
//#include "laupolhumeswidget.h"
#else
#include <QCoreApplication>
#endif
#include "laurobotwidget.h"
#include "laurplidarwidget.h"

#ifdef LAU_SERVER
#include "lautcpserialportwidget.h"
#endif

#ifdef LAU_ROS
#include "lautcprosportwidget.h"
#endif

int main(int argc, char *argv[])
{
#ifdef LAU_CLIENT
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(true);
#else
    QCoreApplication a(argc, argv);
#endif
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));

    //LAUKeyenceWidget w(QString(""));
    //LAURobotWidget w;

#ifdef LAU_ROS
    ros::init(argc, argv, "lautcprosportwidget");
    ros::start();
    LAUTCPROSPortServer s(-1, QString("/realsense/odom"));
#endif

#ifdef LAU_SERVER
    LAURobotServer s(-1, 9220);
    LAURPLidarServer l(-1, 60000);
#endif

#ifdef LAU_CLIENT
    // Serial Port setup
//    LAURobotWidget w(QString(), (QWidget*)NULL);
    // TCP Serial Remote setup
    LAURobotWidget w(QString(), -1, (QWidget*)NULL);
    w.show();
    return a.exec();
#else
    return (a.exec());
#endif
}
