#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T12:50:58
#
#-------------------------------------------------

QT       += core gui

TARGET = ve_projekt
TEMPLATE = app

LIBS += `pkg-config opencv --libs`

QMAKE_CXXFLAGS += `pkg-config opencv --cflags`

INCLUDEPATH += `pkg-config opencv --cflags-only-I` /usr/include/opencv

SOURCES += main.cpp\
        MainWindow.cpp \
    BallTrackingThread.cpp

HEADERS  += MainWindow.h \
    BallTrackingThread.h

FORMS    += MainWindow.ui


RESOURCES += \
    resources.qrc
