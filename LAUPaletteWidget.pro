#**************************************************************************************************
#    Copyright (C) 2017 Dr. Daniel L. Lau
#    This file is part of LAURobotWidget.
#
#    LAURobotWidget is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    LAURobotWidget is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with LAURobotWidget.  If not, see <http://www.gnu.org/licenses/>.
#
#**************************************************************************************************/
CONFIG  -= server
CONFIG  += client
CONFIG  -= ros

QT      += core serialport network

TEMPLATE = app

SOURCES += main.cpp \
           lautcpserialportwidget.cpp \
    laupolhumeswidget.cpp

HEADERS += lautcpserialportwidget.h \
    laupolhumeswidget.h

ros {
    TARGET       = RosServer
    CONFIG      += c++11
    SOURCES     += lautcprosportwidget.cpp
    HEADERS     += lautcprosportwidget.h
    INCLUDEPATH += /opt/ros/kinetic/include
    DEPENDPATH  += /opt/ros/kinetic/include

    unix:!macx {
        DEFINES     += LAU_ROS
        LIBS        += -L/opt/ros/kinetic/lib -lroscpp -lrosconsole \
                       -lroscpp_serialization -lrostime -lrospack \
                       -lrospack -lrosbag -lcpp_common -lxmlrpcpp \
                       -lrosconsole_log4cxx -lrosconsole_backend_interface
    }
}

server {
    TARGET   = RoboServer
    DEFINES += LAU_SERVER
}

client {
    TARGET   = RoboClient
    DEFINES += LAU_CLIENT
    QT      += gui widgets
    SOURCES += laurobotwidget.cpp \
               laupalettewidget.cpp \
               lauzeroconfwidget.cpp \
               laurplidarwidget.cpp \
               lauodomwidget.cpp
    HEADERS += laurobotwidget.h \
               laupalettewidget.h \
               lauzeroconfwidget.h \
               laurplidarwidget.h \
               lauodomwidget.h
    RESOURCES += laupalettegear.qrc
}

# INCLUDE BONJOUR
#include(QtZeroConf/qtzeroconf.pri)
INCLUDEPATH += /usr/local/include
LIBS        += -L/usr/local/lib -lQtZeroConf

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:macx {
    QMAKE_MAC_SDK   = macosx10.12
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
    DEPENDPATH     += /usr/local/include /usr/local/include/eigen3
    LIBS           += /usr/local/lib/libtiff.dylib
}
