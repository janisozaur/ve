#include <vector>
#include <QCoreApplication>
#include <QDebug>
#include <limits>
#include <imgproc/imgproc.hpp>

#include "BallTrackingThread.h"

using namespace cv;

BallTrackingThread::BallTrackingThread(QObject *parent) :
	QThread(parent), mDilutionIterations(0), mErosionIterations(0), mRunning(true),
	mLower(QColor::fromHsv(200, 50, 80)), mUpper(QColor::fromHsv(220, 255, 255)),
	mHue(95), mRadius(10), mSetTRFlag(false), mSetBLFlag(false)
{
	if (!mCapture.open(0)) {
		this->mRunning = false;
	}
	mCapture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	mCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	setLowerBound(mLower);
	setUpperBound(mUpper);
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

void BallTrackingThread::setLowerBound(const QColor &color)
{
	mLower = color;
	mLower.getHsv(&mLowerVec[0], &mLowerVec[1], &mLowerVec[2]);
	mLowerVec[0] /= 2;
	qDebug() << "lower" << mLower.toHsv();
}

void BallTrackingThread::setUpperBound(const QColor &color)
{
	mUpper = color;
	mUpper.getHsv(&mUpperVec[0], &mUpperVec[1], &mUpperVec[2]);
	mUpperVec[0] /= 2;
	qDebug() << "upper" << mUpper.toHsv();
}

Mat BallTrackingThread::GetThresholdedImage(Mat img) const
{
	// Convert the image into an HSV image
	Mat frame;
	cvtColor(img, frame, CV_BGR2HSV);
	Mat threshed;

	if (mHue - mRadius < 0) {
		//qDebug() << "a";
		Mat threshed1;
		Mat threshed2;
#pragma omp parallel
		{
			#pragma omp sections
			{
				#pragma omp section
				{
					cv::Vec3i lowerVec;
					cv::Vec3i upperVec;
					lowerVec = cv::Vec3i(0, 50, 80);
					upperVec = cv::Vec3i(mHue + mRadius, 255, 255);
					inRange(frame, lowerVec, upperVec, threshed1);
				}
				#pragma omp section
				{
					cv::Vec3i lowerVec;
					cv::Vec3i upperVec;
					lowerVec = cv::Vec3i(mHue + 180 - mRadius, 50, 80);
					upperVec = cv::Vec3i(180, 255, 255);
					inRange(frame, lowerVec, upperVec, threshed2);
				}
			}
		}
		cv::bitwise_or(threshed1, threshed2, threshed);
	} else if (mHue + mRadius > 180) {
		//qDebug() << "b";
		Mat threshed1;
		Mat threshed2;
#pragma omp parallel
		{
			#pragma omp sections
			{
				#pragma omp section
				{
					cv::Vec3i lowerVec;
					cv::Vec3i upperVec;
					lowerVec = cv::Vec3i(mHue - mRadius, 50, 80);
					upperVec = cv::Vec3i(180, 255, 255);
					inRange(frame, lowerVec, upperVec, threshed1);
				}
				#pragma omp section
				{
					cv::Vec3i lowerVec;
					cv::Vec3i upperVec;
					lowerVec = cv::Vec3i(0, 50, 80);
					upperVec = cv::Vec3i(mHue + mRadius - 180, 255, 255);
					inRange(frame, lowerVec, upperVec, threshed2);
				}
			}
		}
		cv::bitwise_or(threshed1, threshed2, threshed);
	} else {
		//qDebug() << "c";
		cv::Vec3i lowerVec;
		cv::Vec3i upperVec;
		lowerVec = cv::Vec3i(mHue - mRadius, 50, 80);
		upperVec = cv::Vec3i(mHue + mRadius, 255, 255);
		inRange(frame, lowerVec, upperVec, threshed);
	}
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
		mStorage.resize(0);
		// Show the image captured from the camera in the window and repeat
		// Get one frame
		mCapture >> frame;
		Mat threshed = GetThresholdedImage(frame);
		erode(threshed, threshed, Mat(), Point(-1, -1), mDilutionIterations);
		dilate(threshed, threshed, Mat(), Point(-1, -1), mErosionIterations);

		GaussianBlur(threshed, threshed, Size(9,9), 2, 2);
		HoughCircles(threshed, mStorage, CV_HOUGH_GRADIENT, 2, threshed.cols + threshed.rows, 150, 50);
		//qDebug() << "size" << storage.size();
		for (size_t i = 0; i < mStorage.size(); i++) {
			//qDebug() << "circle:" << storage[i][0] << ", " << storage[i][1] << ", " << storage[i][2];
			circle(frame, Point2f(mStorage[i][0], mStorage[i][1]), mStorage[i][2], color, 2);
			QPoint p(mStorage.at(i)[0], mStorage.at(i)[1]);
			if (mSetTRFlag) {
				mSetTRFlag = false;
				emit setTopRightCorner(p);
			} else if (mSetBLFlag) {
				mSetBLFlag = false;
				emit setBottomLeftCorner(p);
			}
			emit setCurrent(p);
		}
		flip(frame, frame, 1);
		flip(threshed, threshed, 1);
		//cvtColor(frame, frame, CV_RGB2HSV);
		map["mywindow"] = frame.clone();
		map["thresh"] = threshed;
		//qDebug() << "non zero:" << countNonZero(threshed);
		showImage();
		//QCoreApplication::processEvents();
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

int BallTrackingThread::scanEmptyHue(const int &radius) const
{
	Mat frame = map["mywindow"];
	cvtColor(frame, frame, CV_BGR2HSV);
	QVector<int> values(180);
	int *ptr = values.data();
#pragma omp parallel for
	for (int i = 0; i < 180; i++) {
		Mat threshed;
		cv::Vec3i lowerVec;
		cv::Vec3i upperVec;
		int sum = 0;
		if (i - radius < 0) {
			lowerVec = cv::Vec3i(0, 50, 80);
			upperVec = cv::Vec3i(i + radius, 255, 255);
			inRange(frame, lowerVec, upperVec, threshed);
			sum += countNonZero(threshed);
			lowerVec = cv::Vec3i(i + 180 - radius, 50, 80);
			upperVec = cv::Vec3i(180, 255, 255);
			inRange(frame, lowerVec, upperVec, threshed);
			sum += countNonZero(threshed);
		} else if (i + radius > 180) {
			lowerVec = cv::Vec3i(i - radius, 50, 80);
			upperVec = cv::Vec3i(180, 255, 255);
			inRange(frame, lowerVec, upperVec, threshed);
			sum += countNonZero(threshed);
			lowerVec = cv::Vec3i(0, 50, 80);
			upperVec = cv::Vec3i(i + radius - 180, 255, 255);
			inRange(frame, lowerVec, upperVec, threshed);
			sum += countNonZero(threshed);
		} else {
			lowerVec = cv::Vec3i(i - radius, 50, 80);
			upperVec = cv::Vec3i(i + radius, 255, 255);
			inRange(frame, lowerVec, upperVec, threshed);
			sum += countNonZero(threshed);
		}
		ptr[i] = sum;
	}
	int min = std::numeric_limits<int>::max();
	int idx = -1;
	for (int i = 0; i < values.size(); i++) {
		if (values.at(i) < min && i != 90) {
			idx = i;
			min = values.at(i);
		}
	}
	qDebug() << values;
	qDebug() << "the lowest value was at index" << idx << ", it was" << min;
	qDebug() << values.mid(5, 10);
	qDebug() << values.mid(85, 10);
	return idx;
}

void BallTrackingThread::getProperty()
{
	static int i = 0;
	qDebug() << i++;
	qDebug() << "gain" << mCapture.get(CV_CAP_PROP_GAIN)
			 << "exp" << mCapture.get(CV_CAP_PROP_EXPOSURE)
			 << "auto" << mCapture.get(CV_CAP_PROP_AUTO_EXPOSURE);
}

void BallTrackingThread::setBottomLeftCorner()
{
	mSetBLFlag = true;
}

void BallTrackingThread::setTopRightCorner()
{
	mSetTRFlag = true;
}
