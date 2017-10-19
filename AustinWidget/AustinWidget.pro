#-------------------------------------------------
#
# Project created by QtCreator 2017-09-22T13:47:01
#
#-------------------------------------------------

CONFIG  += ros console

QT       += core gui widgets

TARGET = AustinWidget
TEMPLATE = app

SOURCES += main.cpp \
    AustinRosWidget.cpp


ros {
    TARGET       = RosServer
    CONFIG      += c++11
    SOURCES     +=
    HEADERS     +=
    INCLUDEPATH += /opt/ros/kinetic/include
    DEPENDPATH  += /opt/ros/kinetic/include

    unix:!macx {
        DEFINES     += Austin_ROS
        LIBS        += -L/opt/ros/kinetic/lib -lroscpp -lrosconsole \
                       -lroscpp_serialization -lrostime -lrospack \
                       -lrospack -lrosbag -lcpp_common -lxmlrpcpp \
                       -lrosconsole_log4cxx -lrosconsole_backend_interface
    }
}

HEADERS += \
    AustinRosWidget.h


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

#unix:macx {
#    QMAKE_MAC_SDK   = macosx10.12
#    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
#    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
#    #LIBS           += /usr/local/lib/libtiff.5.dylib
#}
