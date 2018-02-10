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

QT      += core gui widgets serialport network

DEFINES += LAU_CLIENT
TARGET   = RoboWidget
TEMPLATE = app

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
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
           laurobotwidget.cpp \
           lauzeroconfwidget.cpp \
           lautcpserialportwidget.cpp

HEADERS += laurobotwidget.h \
           lauzeroconfwidget.h \
           lautcpserialportwidget.h
