#ifndef PSMOVEFORM_H
#define PSMOVEFORM_H

#include "MoveData.h"

#include <QWidget>

namespace Ui {
    class PSMoveForm;
}

class QCheckBox;
class QLabel;
class QStackedLayout;

class PSMoveForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit PSMoveForm(QWidget *parent = 0);
    ~PSMoveForm();
    
public slots:
    void setButtons(MoveButtons buttons);
    void parseMoveData(MoveData d);

signals:
    void setRgb(QColor rgb);
    void setRumble(quint8 rumble);

private slots:
    void on_redDial_valueChanged(int value);

    void on_greenDial_valueChanged(int value);

    void on_blueDial_valueChanged(int value);

private:
    Ui::PSMoveForm *ui;
    QCheckBox *mBatteryCheckBoxes;
    QLabel *mBatteryChargingLabel;
    QStackedLayout *mBatteryLayout;
    QWidget *mBatteryPageWidgets;
    QColor mRGB;
};

#endif // PSMOVEFORM_H
