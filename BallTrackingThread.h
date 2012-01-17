#ifndef BALLTRACKINGTHREAD_H
#define BALLTRACKINGTHREAD_H

#include <QThread>
#include <core/core.hpp>
#include <highgui/highgui.hpp>
#include <QMap>
#include <QColor>

class BallTrackingThread : public QThread
{
    Q_OBJECT
public:
    explicit BallTrackingThread(QObject *parent = 0);
    ~BallTrackingThread();
    void setDilutionIterations(int iterations);
    void setErosionIterations(int iterations);
    QMap<QString, cv::Mat> map;
    QColor getLowerBound() const;
    QColor getUpperBound() const;
    void setLowerBound(const QColor &color);
    void setUpperBound(const QColor &color);

protected:
    void run();

private:
    volatile int mDilutionIterations;
    volatile int mErosionIterations;
    bool mRunning;
    cv::VideoCapture mCapture;
    cv::Mat GetThresholdedImage(cv::Mat img) const;
    QColor mLower;
    QColor mUpper;
    cv::Vec3i mLowerVec;
    cv::Vec3i mUpperVec;
    int mHue;
    int mRadius;
    
signals:
    void showImage();

public slots:
    int scanEmptyHue(const int &spread) const;
};

#endif // BALLTRACKINGTHREAD_H
