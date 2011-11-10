#ifndef MOVEDATA_H
#define MOVEDATA_H

#include "MoveButtons.h"

#include <QVector3D>

struct MoveData {
    MoveButtons buttons;
    quint8 battery;
    quint8 temperature;
    QVector3D accelerometer;
    QVector3D gyro;
    QVector3D mag;
};

#endif // MOVEDATA_H
