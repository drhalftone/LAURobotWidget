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

#include "laukeyencewidget.h"
#include "laurobotwidget.h"
#include "laurplidarwidget.h"
#include "lautcpserialportwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setQuitOnLastWindowClosed(true);

    //LAUKeyenceWidget w(QString(""));
    //LAURobotWidget w;

    LAUTCPSerialPortServer s(-1, 60000);
    LAURPLidarDialog w(QString(), -1, NULL);
    if (w.isValid()) {
        return (w.exec());
    }
    return (0);
}
