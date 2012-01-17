#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T12:50:58
#
#-------------------------------------------------

QT       += core gui opengl

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
    MoveThread.cpp \
    IirFilter.cpp \
    DisplayWidget.cpp \
    GLForm.cpp \
    MadgwickAHRS.cpp \
    Displayer.cpp \
    Model.cpp \
    GLDebugDrawer.cpp \
    GLDebugFont.cpp \
    FPSCounter.cpp

HEADERS  += MainWindow.h \
    BallTrackingThread.h \
    PSMoveForm.h \
    MoveButtons.h \
    MoveData.h \
    MoveThread.h \
    IirFilter.h \
    DisplayWidget.h \
    GLForm.h \
    MadgwickAHRS.h \
    Displayer.h \
    Model.h \
    TextureInfo.h \
    GLDebugDrawer.h \
    GLDebugFont.h \
    FPSCounter.h

FORMS    += MainWindow.ui \
    PSMoveForm.ui \
    GLForm.ui

RESOURCES += \
    resources.qrc
