#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "BallTrackingThread.h"
#include "MoveThread.h"
#include "PSMoveForm.h"
#include "MoveButtons.h"
#include "GLForm.h"
#include "Displayer.h"
#include "WiiMarkerTracker.h"
#include "SpeechRecognition.h"

#include <highgui/highgui.hpp>
#include <QColorDialog>
#include <GL/freeglut.h>
#include <QTimer>

#include <QDebug>

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mBallTracker(nullptr), mMoveThread(nullptr), mMoveForm(nullptr), mWMT(nullptr), mGLForm(nullptr), mDisplayer(nullptr), mSpeech(nullptr)
{
	ui->setupUi(this);
	qRegisterMetaType<MoveButtons>("MoveButtons");
	//cv::namedWindow( "mywindow", CV_WINDOW_AUTOSIZE );
	//cv::namedWindow( "thresh", CV_WINDOW_AUTOSIZE );

	int argc = 0;
	char *argv = nullptr;
	glutInit(&argc, &argv);
//	mGLForm = new GLForm();
//	mGLForm->show();
	mDisplayer = new Displayer();
	mDisplayer->show();
	mWMT = new WiiMarkerTracker(&mWiiSemaphore, qobject_cast<QObject *>(this));
	mWMT->start();
	connect(mWMT, SIGNAL(newPosition(QPointF)), mDisplayer, SLOT(setRelativeCameraPos(QPointF)));
	QTimer *t = new QTimer(this);
	t->start(50);
	connect(t, SIGNAL(timeout()), this, SLOT(releaseWiiSemaphore()));
	mSpeech = new SpeechRecognition(this);
	bool sp = mSpeech->init("/dev/dsp");
	qDebug() << "speech:" << sp;
	connect(mSpeech, SIGNAL(fullscreen()), mDisplayer, SLOT(showFullScreen()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
	mBallTracker = new BallTrackingThread();
	connect(mBallTracker, SIGNAL(showImage()), this, SLOT(showImage()));
	mBallTracker->start();
	ui->pushButton->setEnabled(false);
	connectSignals();
}

void MainWindow::showImage()
{
	for (int i = 0; i < mBallTracker->map.size(); i++) {
		const std::string window = mBallTracker->map.keys().at(i).toStdString();
		//qDebug() << "painting to " << window;
		cv::imshow(window, mBallTracker->map.values().at(i));
	}
}

void MainWindow::on_setLowerPushButton_clicked()
{
	if (mBallTracker == NULL) {
		return;
	}
	QColor c = QColorDialog::getColor(mBallTracker->getLowerBound());
	if (c.isValid()) {
		mBallTracker->setLowerBound(c);
	}
}

void MainWindow::on_setUpperPushButton_clicked()
{
	if (mBallTracker == NULL) {
		return;
	}
	QColor c = QColorDialog::getColor(mBallTracker->getUpperBound());
	if (c.isValid()) {
		mBallTracker->setUpperBound(c);
	}
}

void MainWindow::on_erosionSlider_valueChanged(int value)
{
	if (mBallTracker != NULL) {
		mBallTracker->setErosionIterations(value);
	}
}

void MainWindow::on_dilutionSlider_valueChanged(int value)
{
	if (mBallTracker != NULL) {
		mBallTracker->setDilutionIterations(value);
	}
}

void MainWindow::on_moveConnectPushButton_clicked()
{
	mMoveForm = new PSMoveForm();
	mMoveForm->show();
	if (mGLForm != nullptr) {
		connect(mMoveForm, SIGNAL(setVector(QVector3D)), mGLForm, SLOT(setVector(QVector3D)));
		connect(mMoveForm, SIGNAL(setMatrix(QMatrix4x4)), mGLForm, SLOT(setMatrix(QMatrix4x4)));
	} else {
		qDebug() << "there is no gl form set, not connecting signals";
	}
	mMoveThread = new MoveThread(&mMoveSemaphore, this);
	connect(mMoveThread, SIGNAL(dataReceived(MoveData)), mDisplayer, SLOT(receiveData(MoveData)));
	connect(mMoveThread, SIGNAL(dataReceived(MoveData)), mMoveForm, SLOT(parseMoveData(MoveData)));
	connect(mMoveForm, SIGNAL(setRgb(QColor)), mMoveThread, SLOT(setRGB(QColor)));
	connect(mMoveForm, SIGNAL(setRumble(quint8)), mMoveThread, SLOT(setRumble(quint8)));
	mMoveThread->start();
	QTimer *t = new QTimer(this);
	t->start(30);
	connect(t, SIGNAL(timeout()), this, SLOT(releaseMoveSemaphore()));
	connectSignals();
}

void MainWindow::connectSignals()
{
	if (mMoveThread != nullptr) {
		connect(mMoveThread, SIGNAL(outputCurrent()), mDisplayer, SLOT(outputCurrent()));
	}
	if (mBallTracker != nullptr && mMoveThread != nullptr) {
		connect(mMoveThread, SIGNAL(startClicked()), mBallTracker, SLOT(getProperty()));
		connect(mMoveForm, SIGNAL(setTopRightCorner()), mBallTracker, SLOT(setTopRightCorner()));
		connect(mMoveForm, SIGNAL(setBottomLeftCorner()), mBallTracker, SLOT(setBottomLeftCorner()));
		connect(mBallTracker, SIGNAL(setTopRightCorner(QPointF)), mDisplayer, SLOT(setTopRightCorner(QPointF)));
		connect(mBallTracker, SIGNAL(setBottomLeftCorner(QPointF)), mDisplayer, SLOT(setBottomLeftCorner(QPointF)));
		connect(mBallTracker, SIGNAL(setCurrent(QPointF)), mDisplayer, SLOT(setCurrent(QPointF)));
	}
}

void MainWindow::on_scanPushButton_clicked()
{
	if (mBallTracker == NULL) {
		return;
	}
	int radius = 10;
	int emptyHue = mBallTracker->scanEmptyHue(radius);
	const QColor lower = QColor::fromHsv(emptyHue - radius, 50, 80);
	const QColor upper = QColor::fromHsv(emptyHue + radius, 255, 255);
	mBallTracker->setLowerBound(lower);
	mBallTracker->setUpperBound(upper);
}

void MainWindow::releaseMoveSemaphore()
{
	mMoveSemaphore.release();
}

void MainWindow::releaseWiiSemaphore()
{
	if (mWMT != nullptr && mWMT->isRunning()) {
		mWiiSemaphore.release();
	}
}
