#include "PSMoveForm.h"
#include "ui_PSMoveForm.h"

#include <QStackedLayout>
#include <QCheckBox>
#include <QHBoxLayout>

#include <QDebug>

PSMoveForm::PSMoveForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSMoveForm)
{
    ui->setupUi(this);
    mBatteryLayout = new QStackedLayout(ui->batteryContainerWidget);
    mBatteryCheckBoxes = new QCheckBox [5];
    for (int i = 0; i < 5; i++) {
        mBatteryCheckBoxes[i].setParent(ui->batteryContainerWidget);
    }
    mBatteryChargingLabel = new QLabel("Charging", ui->batteryContainerWidget);
    mBatteryPageWidgets = new QWidget [2];
    mBatteryPageWidgets[0].setParent(ui->batteryContainerWidget);
    mBatteryPageWidgets[1].setParent(ui->batteryContainerWidget);
    mBatteryPageWidgets[0].setLayout(new QHBoxLayout);
    mBatteryPageWidgets[1].setLayout(new QHBoxLayout);
    for (int i = 0; i < 5; i++) {
        mBatteryCheckBoxes[i].setEnabled(false);
        mBatteryPageWidgets[0].layout()->addWidget(static_cast<QWidget *>(&mBatteryCheckBoxes[i]));
    }
    mBatteryPageWidgets[1].layout()->addWidget(mBatteryChargingLabel);
    mBatteryLayout->addWidget(&mBatteryPageWidgets[0]);
    mBatteryLayout->addWidget(&mBatteryPageWidgets[1]);
}

PSMoveForm::~PSMoveForm()
{
    delete [] mBatteryCheckBoxes;
    delete ui;
}

void PSMoveForm::setButtons(MoveButtons buttons)
{
    //qDebug() << "setting buttons";
    //qDebug() << "recv: " << buttons.buttonsPressed;
    ui->circlePushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Circle));
    ui->squarePushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Square));
    ui->trianglePushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Triangle));
    ui->crossPushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Cross));
    ui->movePushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Move));
    ui->psPushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::PS));
    ui->selectPushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Select));
    ui->startPushButton->setChecked(buttons.buttonsPressed.contains(MoveButtons::Start));
    ui->triggerHorizontalSlider->setValue(buttons.trigger);
}

void PSMoveForm::parseMoveData(MoveData d)
{
    setButtons(d.buttons);
    ui->tempLcdNumber->display(d.temperature);
    if (d.battery <= 5) {
        for (int i = 0; i < 5; i++) {
            mBatteryCheckBoxes[i].setChecked(i < d.battery);
        }
        mBatteryLayout->setCurrentIndex(0);
    } else {
        mBatteryLayout->setCurrentIndex(1);
    }
    emit setRumble(d.buttons.trigger);
    /*const QVector3D acc = d.accelerometer.normalized() * 128 + QVector3D(128, 128, 128);
    const QColor rgb(acc.x(), acc.y(), acc.z());
    static int w = 0;
    if (w++ == 10) {
        w = 0;
        emit setRgb(rgb);
    }*/
    if (ui->accGroupBox->isChecked()) {
        ui->accXProgressBar->setValue(d.accelerometer.x());
        ui->accYProgressBar->setValue(d.accelerometer.y());
        ui->accZProgressBar->setValue(d.accelerometer.z());
    }

    if (ui->gyroGroupBox->isChecked()) {
        ui->gyroXProgressBar->setValue(d.gyro.x());
        ui->gyroYProgressBar->setValue(d.gyro.y());
        ui->gyroZProgressBar->setValue(d.gyro.z());
    }
}

void PSMoveForm::on_redDial_valueChanged(int value)
{
    mRGB.setRed(value);
    emit setRgb(mRGB);
}

void PSMoveForm::on_greenDial_valueChanged(int value)
{
    mRGB.setGreen(value);
    emit setRgb(mRGB);
}

void PSMoveForm::on_blueDial_valueChanged(int value)
{
    mRGB.setBlue(value);
    emit setRgb(mRGB);
}
