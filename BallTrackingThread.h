#ifndef BALLTRACKINGTHREAD_H
#define BALLTRACKINGTHREAD_H

#include <QThread>
#include <cv.h>
#include <highgui.h>
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
    void setLowerBound(const QColor color);
    void setUpperBound(const QColor color);

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
    
signals:
    void showImage();

public slots:
};

#endif // BALLTRACKINGTHREAD_H
