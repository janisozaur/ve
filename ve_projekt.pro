#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T12:50:58
#
#-------------------------------------------------

QT       += core gui

TARGET = ve_projekt
TEMPLATE = app

LIBS += `pkg-config opencv --libs` -lgomp

QMAKE_CXXFLAGS += -std=gnu++0x -fopenmp
QMAKE_CXXFLAGS += `pkg-config opencv --cflags`

INCLUDEPATH += `pkg-config opencv --cflags-only-I` /usr/include/opencv

SOURCES += main.cpp\
        MainWindow.cpp \
    BallTrackingThread.cpp \
    PSMoveForm.cpp

HEADERS  += MainWindow.h \
    BallTrackingThread.h \
    PSMoveForm.h \
    MoveButtons.h \
    MoveData.h

FORMS    += MainWindow.ui \
    PSMoveForm.ui

RESOURCES += \
    resources.qrc
