#include "PSMoveForm.h"
#include "ui_PSMoveForm.h"
#include "MadgwickAHRS.h"
#include "IirFilter.h"

#include <QStackedLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <cmath>
#include <QQuaternion>
#include <QMatrix4x4>
#include <GL/freeglut.h>

#include <QDebug>

PSMoveForm::PSMoveForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::PSMoveForm),
	mPrevMovePressed(false),
	mPrevTriPressed(false),
	mPrevSquPressed(false)
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

	double k = 0.18;
	double coeffs[2] = { 1, -(1-k) };
	for (int i = 0; i < 3; i++) {
		mAccFilters[i] = new IirFilter(&k, 1, coeffs, 2);
		mMagFilters[i] = new IirFilter(&k, 1, coeffs, 2);
	}
}

PSMoveForm::~PSMoveForm()
{
	for (int i = 0; i < 3; i++) {
		delete mAccFilters[i];
		delete mMagFilters[i];
	}
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
	//emit setRumble(d.buttons.trigger);
	/*const QVector3D acc = d.accelerometer.normalized() * 128 + QVector3D(128, 128, 128);
	const QColor rgb(acc.x(), acc.y(), acc.z());
	static int w = 0;
	if (w++ == 10) {
		w = 0;
		emit setRgb(rgb);
	}*/

	//QVector3D normalisedAcc = d.accelerometer / 32768;
	//QVector3D normalisedMag = d.mag / 32768;
	//normalisedAcc.normalize();
	//normalisedMag = QVector3D::crossProduct(normalisedMag.normalized(), normalisedAcc).normalized();
	//QVector3D north = QVector3D::crossProduct(normalisedMag, normalisedAcc).normalized();

	bool movePressedB = d.buttons.buttonsPressed.contains(MoveButtons::Move);
	if (!mPrevMovePressed && movePressedB) {

		//mOne = normalisedAcc;
		//mTwo = normalisedMag;
		//mThree = north;

		qDebug() << "acc:" << d.accelerometer;
		qDebug() << "mag:" << d.mag;
		emit movePressed();
	}
	mPrevMovePressed = movePressedB;
	/*float one = std::acos(QVector3D::dotProduct(normalisedAcc, mOne));
	float two = std::acos(QVector3D::dotProduct(normalisedMag, mTwo));
	float three = std::acos(QVector3D::dotProduct(north, mThree));*/
	//qDebug() << QString::number(one, 'g', 4) << QString::number(two, 'g', 4) << QString::number(three, 'g', 4);
	/*ui->accXProgressBar->setValue(one * 180 / M_PI);
	ui->accYProgressBar->setValue(two * 180 / M_PI);
	ui->accZProgressBar->setValue(three * 180 / M_PI);*/

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

	if (ui->magGroupBox->isChecked()) {
		ui->magXProgressBar->setValue(d.mag.x());
		ui->magYProgressBar->setValue(d.mag.y());
		ui->magZProgressBar->setValue(d.mag.z());
	}

	bool triPressed = d.buttons.buttonsPressed.contains(MoveButtons::Triangle);
	if (triPressed && !mPrevTriPressed) {
		emit setTopRightCorner();
	}
	mPrevTriPressed = triPressed;

	bool squPressed = d.buttons.buttonsPressed.contains(MoveButtons::Square);
	if (squPressed && !mPrevSquPressed) {
		emit setBottomLeftCorner();
	}
	mPrevSquPressed = squPressed;
#if 0
	QVector3D acc = d.accelerometer;
	QVector3D mag = d.mag;
	acc.normalize();
	mag.normalize();
	QMatrix4x4 m;
	m.setRow(2, acc);
	QVector3D east = QVector3D::crossProduct(acc, -mag);
	m.setRow(0, east);
	QVector3D north = QVector3D::crossProduct(acc, -mag);
	m.setRow(1, north);
	//qDebug() << mag;

//    QVector3D gyro = d.gyro / 32768;
//    acc.setX(mAccFilters[0]->step(acc.x()));
//    acc.setY(mAccFilters[1]->step(acc.y()));
//    acc.setZ(mAccFilters[2]->step(acc.z()));

//    mag.setX(mMagFilters[0]->step(mag.x()));
//    mag.setY(mMagFilters[1]->step(mag.y()));
//    mag.setZ(mMagFilters[2]->step(mag.z()));

	QVector3D newY = acc;
	QVector3D newZ = mag;
	newZ.setZ(-newZ.z());
	QVector3D newX = QVector3D::crossProduct(newZ, newY).normalized();
	QMatrix4x4 newMat;
	newMat.setRow(0, newX);
	newMat.setRow(1, newY);
	newMat.setRow(2, newZ);
	newMat.setRow(3, QVector4D(0, 0, 0, 1));
	float qq1 = q0;
	float qq2 = q1;
	float qq3 = q2;
	float qq4 = q3;
	QQuaternion quat(qq1, qq2, qq3, qq4);
	QVector3D vec(1, 1, 1);
	emit setMatrix(newMat);
	//emit setVector(quat.rotatedVector(vec));
#endif
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
