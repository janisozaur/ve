#include <vector>
#include <QCoreApplication>
#include <QDebug>

#include "BallTrackingThread.h"

using namespace cv;

BallTrackingThread::BallTrackingThread(QObject *parent) :
    QThread(parent), mDilutionIterations(0), mErosionIterations(0), mRunning(true)
{
    if (!mCapture.open(0)) {
        this->mRunning = false;
    }
    qDebug() << "running:" << mRunning;
}

QColor BallTrackingThread::getLowerBound() const
{
    return mLower;
}

QColor BallTrackingThread::getUpperBound() const
{
    return mUpper;
}

void BallTrackingThread::setLowerBound(const QColor color)
{
    mLower = color;
}

void BallTrackingThread::setUpperBound(const QColor color)
{
    mUpper = color;
}

Mat BallTrackingThread::GetThresholdedImage(Mat img) const
{
    // Convert the image into an HSV image
    Mat imgHSV;
    cvtColor(img, imgHSV, CV_BGR2HSV);
    Mat threshed;
    Vec3f lower;
    // BGR
    lower[0] = mErosionIterations;
    lower[1] = 30;
    lower[2] = 80;

    Vec3f upper;
    // BGR
    upper[0] = mDilutionIterations;
    upper[1] = 255;
    upper[2] = 255;

    inRange(imgHSV, lower, upper, threshed);
    return threshed;
}

void BallTrackingThread::run()
{
    qDebug() << "run";
    qDebug() << "starting";
    Scalar_<float> color = CV_RGB(255, 255, 0);
    while (mRunning) {
        //qDebug() << "beep" << mDilutionIterations << mErosionIterations;
        Mat frame;
        mCapture >> frame;
        // Show the image captured from the camera in the window and repeat
        // Get one frame
        mCapture >> frame;
        Mat threshed = GetThresholdedImage(frame);
        std::vector<Vec3f> storage;
        //erode(threshed, threshed, 0, Point(-1, -1), mDilutionIterations);
        //dilate(threshed, threshed, 0, Point(-1, -1), mErosionIterations);

        GaussianBlur(threshed, threshed, Size(7,7), 1.5, 1.5);
        HoughCircles(threshed, storage, CV_HOUGH_GRADIENT, 2, threshed.cols, 200, 100);
        //qDebug() << "size" << storage.size();
        for (size_t i = 0; i < storage.size(); i++) {
            //qDebug() << "circle:" << storage[i][0] << ", " << storage[i][1] << ", " << storage[i][2];
            circle(frame, Point2f(storage[i][0], storage[i][1]), storage[i][2], color, 2);
        }
        flip(frame, frame, 1);
        flip(threshed, threshed, 1);
        //cvtColor(frame, frame, CV_RGB2HSV);
        map["mywindow"] = frame;
        map["thresh"] = threshed;
        showImage();
        QCoreApplication::processEvents();
    }
}

BallTrackingThread::~BallTrackingThread()
{
    mCapture.release();
}

void BallTrackingThread::setDilutionIterations(int iterations)
{
    mDilutionIterations = iterations;
    qDebug() << "changed dilution: " << mDilutionIterations;
}

void BallTrackingThread::setErosionIterations(int iterations)
{
    mErosionIterations = iterations;
    qDebug() << "changed erosion: " <<  mErosionIterations;
}
