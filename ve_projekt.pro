#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T12:50:58
#
#-------------------------------------------------

QT       += core gui

TARGET = ve_projekt
TEMPLATE = app

LIBS += -lbluetooth -lgomp -lGLU -lglut -l3ds
PKGCONFIG += bullet opencv

QMAKE_CXXFLAGS += -std=gnu++0x -fopenmp

INCLUDEPATH += /usr/include/opencv2 /usr/local/include/bullet/

SOURCES += main.cpp\
        MainWindow.cpp \
    BallTrackingThread.cpp \
    PSMoveForm.cpp \
    MoveThread.cpp

HEADERS  += MainWindow.h \
    BallTrackingThread.h \
    PSMoveForm.h \
    MoveButtons.h \
    MoveData.h \
    MoveThread.h

FORMS    += MainWindow.ui \
    PSMoveForm.ui

RESOURCES += \
    resources.qrc
