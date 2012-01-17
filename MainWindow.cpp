#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "BallTrackingThread.h"
#include "MoveThread.h"
#include "PSMoveForm.h"
#include "MoveButtons.h"
#include "GLForm.h"
#include "Displayer.h"

#include <highgui/highgui.hpp>
#include <QColorDialog>

#include <QDebug>


using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
    ui(new Ui::MainWindow),
    mBallTracker(nullptr), mMoveThread(nullptr), mMoveForm(nullptr)
{
	ui->setupUi(this);
    qRegisterMetaType<MoveButtons>("MoveButtons");
    //cv::namedWindow( "mywindow", CV_WINDOW_AUTOSIZE );
    //cv::namedWindow( "thresh", CV_WINDOW_AUTOSIZE );

    mGLForm = new GLForm();
    mGLForm->show();
	mDisplayer = new Displayer();
	mDisplayer->show();
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
    connect(mMoveForm, SIGNAL(setVector(QVector3D)), mGLForm, SLOT(setVector(QVector3D)));
    connect(mMoveForm, SIGNAL(setMatrix(QMatrix4x4)), mGLForm, SLOT(setMatrix(QMatrix4x4)));
    mMoveThread = new MoveThread(this);
    connect(mMoveThread, SIGNAL(dataReceived(MoveData)), mMoveForm, SLOT(parseMoveData(MoveData)));
    connect(mMoveForm, SIGNAL(setRgb(QColor)), mMoveThread, SLOT(setRGB(QColor)));
    connect(mMoveForm, SIGNAL(setRumble(quint8)), mMoveThread, SLOT(setRumble(quint8)));
    mMoveThread->start();
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
