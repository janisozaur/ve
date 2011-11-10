#ifndef MOVEBUTTONS_H
#define MOVEBUTTONS_H

#include <QMetaType>
#include <QVector>

struct MoveButtons {
    enum Button {
        Square,
        Triangle,
        Cross,
        Circle,
        Move,
        PS,
        Start,
        Select
    };
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAGS(Button Buttons)
    QVector<Button> buttonsPressed;
    quint8 trigger;
};

Q_DECLARE_METATYPE(MoveButtons)
Q_DECLARE_OPERATORS_FOR_FLAGS(MoveButtons::Buttons)

#endif // MOVEBUTTONS_H
