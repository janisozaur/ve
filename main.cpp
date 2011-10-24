#include "MainWindow.h"

#include <QtGui/QApplication>
#include <QDebug>
#include <cv.h>
#include <highgui.h>
#include <vector>

using namespace cv;
using namespace std;

Mat GetThresholdedImage(Mat img)
{
    // Convert the image into an HSV image
    Mat imgHSV;
    cvtColor(img, imgHSV, CV_RGB2HSV);
    Mat threshed;
    Vec3f lower;
    lower[0] = 15;
    lower[1] = 30;
    lower[2] = 80;
    Vec3f upper;
    upper[0] = 25;
    upper[1] = 255;
    upper[2] = 255;
    inRange(imgHSV, lower, upper, threshed);
    return threshed;
}

int main(int argc, char *argv[])
{
    cv::VideoCapture capture;
    if (!capture.open(0)) {
        throw QString("Unable to open device");
        return 1;
    }
    // Create a window in which the captured images will be presented
    namedWindow( "mywindow", CV_WINDOW_AUTOSIZE );
    namedWindow( "thresh", CV_WINDOW_AUTOSIZE );
    Mat frame;
    capture >> frame;
    qDebug() << IplImage(frame).colorModel;
    Scalar_<float> color = CV_RGB(255, 255, 0);
    // Show the image captured from the camera in the window and repeat
    while ( 1 ) {
        // Get one frame
        capture >> frame;
        Mat threshed = GetThresholdedImage(frame);
        vector<Vec3f> storage;
        erode(threshed, threshed, 0, Point(-1, -1), 5);
        dilate(threshed, threshed, 0, Point(-1, -1), 5);
        GaussianBlur(threshed, threshed, Size(7,7), 1.5, 1.5);
        HoughCircles(threshed, storage, CV_HOUGH_GRADIENT, 2, threshed.cols, 200, 100);
        //qDebug() << "size" << storage.size();
        for (size_t i = 0; i < storage.size(); i++) {
            //qDebug() << "circle:" << storage[i][0] << ", " << storage[i][1] << ", " << storage[i][2];
            circle(frame, Point2f(storage[i][0], storage[i][1]), storage[i][2], color, 2);
        }
        flip(frame, frame, 1);
        flip(threshed, threshed, 1);
        imshow( "mywindow", frame );
        imshow( "thresh", threshed );
        //If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
        //remove higher bits using AND operator
        if ( (cvWaitKey(10) & 255) == 27 ) break;
    }
    capture.release();
    return 0;
}
