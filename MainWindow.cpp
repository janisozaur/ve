#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <highgui.h>
#include <QColorDialog>

#include <QDebug>

#include "BallTrackingThread.h"

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
    ui(new Ui::MainWindow),
    mBallTracker(NULL)
{
	ui->setupUi(this);
    cv::namedWindow( "mywindow", CV_WINDOW_AUTOSIZE );
    cv::namedWindow( "thresh", CV_WINDOW_AUTOSIZE );
    qRegisterMetaType<Mat>("Mat");
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
    mBallTracker->setLowerBound(c);
}

void MainWindow::on_setUpperPushButton_clicked()
{
    if (mBallTracker == NULL) {
        return;
    }
    QColor c = QColorDialog::getColor(mBallTracker->getUpperBound());
    mBallTracker->setUpperBound(c);
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
